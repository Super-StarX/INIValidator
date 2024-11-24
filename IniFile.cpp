#include "IniFile.h"
#include "Log.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <algorithm>
#include <filesystem>

IniFile::IniFile(const std::string& filepath) {
    load(filepath);
}

void IniFile::load(const std::string& filepath) {
    if (!std::filesystem::exists(filepath))
        throw std::runtime_error("File not found: " + filepath);

    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + filepath);

    std::string line, currentSection;
    int lineNumber = 0;

    // 逐行扫描加载ini
    while (std::getline(file, line)) {
        lineNumber++;
        // 移除注释之后的东西,并掐头去尾
        line = removeInlineComment(line);
        line = trim(line);
        if (line.empty()) continue;

        if (line[0] == '[')
            readSection(line, lineNumber, currentSection);
        else if (!currentSection.empty())
            readKeyValue(currentSection, line, lineNumber);
    }
    processIncludes(std::filesystem::path(filepath).parent_path().string());
    processInheritance();
}

// 开头是[则为节名
void IniFile::readSection(std::string& line, int& lineNumber, std::string& currentSection) {
    size_t endPos = line.find(']');
    if (endPos == std::string::npos)
        ERROR(lineNumber) << "No closing ']' found.";
    else {
        currentSection = line.substr(1, endPos - 1);
        if (endPos != line.size() - 1) // 检查 ']' 是否是最后一个字符
            INFO(lineNumber) << "']' is not the last character.";
    }
}

// 读取键值对
void IniFile::readKeyValue(std::string& currentSection, std::string& line, int lineNumber) {
    auto& section = sections[currentSection];
    auto delimiterPos = line.find('=');
    if (delimiterPos != std::string::npos) {
        // 传统键值对
        std::string key = trim(line.substr(0, delimiterPos));
        std::string value = trim(line.substr(delimiterPos + 1));
		// += 的特殊处理
		if (key == "+") {
			static int var_num = 0;
			key = "var_" + std::to_string(var_num);
			++var_num;
		}
		else if (section.count(key))
			WARNING(lineNumber) << "Duplicate key \"" << key << "\" in " << currentSection << ", value \"" << section[key] << "\" overwritten by \"" << value << "\".";
		section[key] = { value, lineNumber };
    }
    else {
        // 仅有键, 无值, 用于配置ini的注册表, 暂时不报错, 未来会改
        section[line] = { "", lineNumber };
    }
}

// 处理#include
void IniFile::processIncludes(const std::string& basePath) {
    // 找到名为#include的节
    if (sections.count("#include")) {
        // 遍历#include里的所有键值对
        for (const auto& [key, value] : sections["#include"]) {
            // 对于每一个值,找到其对应的ini读进来接到sections里
            IniFile includedFile(basePath + "/" + value.value);
            for (const auto& [sec, keyvalue] : includedFile.sections)
                sections[sec].insert(keyvalue.begin(), keyvalue.end());
        }
    }
}

// 处理[]:[]
void IniFile::processInheritance() {
    std::vector<std::string> sectionsToProcess;
    for (const auto& [section, keys] : sections)
        sectionsToProcess.push_back(section);

    for (const std::string& sectionName : sectionsToProcess) {
        auto pos = sectionName.find(':');
        if (pos != std::string::npos) {
            std::string section = sectionName.substr(0, pos);
            std::string parent = sectionName.substr(pos + 1);

            if (sections.count(parent))
                sections[section].insert(sections[parent].begin(), sections[parent].end());
        }
    }
}

// 去除注释
std::string IniFile::removeInlineComment(const std::string& str) {
    size_t commentPos = str.find(';');
    return commentPos != std::string::npos ? str.substr(0, commentPos) : str;
}

// 去除字符串开头结尾
std::string IniFile::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}