#include "IniFile.h"
#include "Log.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <algorithm>
#include <filesystem>

static std::vector<std::string> FileNames;

IniFile::IniFile(const std::string& filepath, bool isConfig) :isConfig(isConfig) {
    load(filepath);
}

char fileIndex = -1;
void IniFile::load(const std::string& filepath) {
	if (!std::filesystem::exists(filepath)) {
		LOG << "File not found: " << filepath;
		return;
	}

    std::ifstream file(filepath);
    if (!file.is_open()) {
		LOG << "Failed to open file: " << filepath;
		return;
	}

	FileNames.push_back(std::filesystem::path(filepath).filename().string());
	fileIndex++;
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
}

// 开头是[则为节名
void IniFile::readSection(std::string& line, int& lineNumber, std::string& currentSection) {
    size_t endPos = line.find(']');
	if (endPos == std::string::npos)
		ERRORF(currentSection, FileNames.back(), lineNumber) << "中括号未闭合";
    else {
        currentSection = line.substr(1, endPos - 1);
		sections[currentSection].name = currentSection;
		processInheritance(line, endPos, lineNumber, currentSection);
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
			WARNINGK(section, key) << "重复的键，\"" << value << "\"被覆盖为\"" << section[key] << "\".";
		section[key] = Value{ value, lineNumber,fileIndex };
    }
    else {
        // 仅有键, 无值, 用于配置ini的注册表, 暂时不报错, 未来会改
        section[line] = Value{ "", lineNumber,fileIndex };
    }
}

// 处理#include
void IniFile::processIncludes(const std::string& basePath) {
    // 找到名为#include的节
    if (sections.count("#include")) {
        // 遍历#include里的所有键值对
        for (const auto& [key, value] : sections["#include"]) {
            // 对于每一个值,找到其对应的ini读进来接到sections里
            IniFile includedFile(basePath + "/" + value.value, isConfig);
            for (const auto& [sec, keyvalue] : includedFile.sections)
                sections[sec].insert(keyvalue);
        }
    }
}

// 处理[]:[]
void IniFile::processInheritance(std::string& line, size_t endPos, int& lineNumber, std::string& currentSection) {
	size_t colonPos = line.find(':', endPos + 1);
	if (colonPos != std::string::npos) {
		// 检查 ':' 之后的第一个字符是否是 '['
		if (colonPos + 1 < line.size() && line[colonPos + 1] == '[') {
			size_t nextEndPos = line.find(']', colonPos + 2);
			if (nextEndPos == std::string::npos) {
				ERRORF(currentSection, FileNames.back(), lineNumber) << "继承对象的中括号未闭合";
				return;
			}

			std::string inheritedSections = line.substr(colonPos + 2, nextEndPos - colonPos - 2);
			if (sections.count(inheritedSections))
				sections[currentSection].insert(sections[inheritedSections]);
			else
				ERRORF(currentSection, FileNames.back(), lineNumber) << "继承的节：\"" << inheritedSections << "\"未找到";
		}
		else
			WARNINGF(currentSection, FileNames.back(), lineNumber) << "继承格式不正确";
	}
	else if (endPos != line.size() - 1) // 检查 ']' 是否是最后一个字符
		INFOF(currentSection, FileNames.back(), lineNumber) << "未以']'结尾";
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

std::string Value::getFileName() const {
	return FileNames.at(fileIndex); 
}
