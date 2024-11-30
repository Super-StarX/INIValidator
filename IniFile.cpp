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

std::string IniFile::GetFileName(size_t index) {
	return FileNames.at(index);
}

IniFile::IniFile(const std::string& filepath, bool isConfig) :isConfig(isConfig) {
    load(filepath);
}

char fileIndex = -1;
void IniFile::load(const std::string& filepath) {
	auto path = std::regex_replace(filepath, std::regex("^\"|\"$"), "");

	if (!std::filesystem::exists(path)) {
		LOG << "File not found: " << path;
		return;
	}

    std::ifstream file(path);
    if (!file.is_open()) {
		LOG << "Failed to open file: " << path;
		return;
	}

	FileNames.push_back(std::filesystem::path(path).filename().string());
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
    processIncludes(std::filesystem::path(path).parent_path().string());
}

// 开头是[则为节名
void IniFile::readSection(std::string& line, int& lineNumber, std::string& currentSection) {
    size_t endPos = line.find(']');
	if (endPos == std::string::npos)
		ERRORF(currentSection, FileNames.size() - 1, lineNumber) << "中括号未闭合";
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
		auto key = trim(line.substr(0, delimiterPos));
		auto value = trim(line.substr(delimiterPos + 1));
		// += 的特殊处理
		if (key == "+") {
			static int var_num = 0;
			key = "var_" + std::to_string(var_num);
			++var_num;
		}
		else if (section.count(key)) {
			auto& oldValue = section[key];
			// 如果现存的值是继承来的值，则不报警，新值覆盖后会去掉继承标签
			// 如果是同文件内的覆盖, 就报警, 跨文件不报
			if (!oldValue.isInheritance && oldValue.fileIndex == fileIndex)
				WARNINGF(section.name, oldValue.fileIndex, lineNumber) << "\"" << key << "\"重复设定，"
					<< "第" << oldValue.line << "行的\"" << oldValue
					<< "\"被覆盖为\"" << value << "\"。";
		}
		section[key] = { value, lineNumber, fileIndex };
    }
    else {
        // 仅有键, 无值, 用于配置ini的注册表, 暂时不报错, 未来会改
        section[line] = { "", lineNumber, fileIndex };
    }
}

// 处理#include
void IniFile::processIncludes(const std::string& basePath) {
    // 找到名为#include的节
    if (sections.count("#include")) {
        // 遍历#include里的所有键值对，因为unordered_map没有顺序，所以重新按顺序遍历
		int curFileIndex = fileIndex;
		auto& section = sections["#include"];
		using MapType = decltype(section.section);
		std::vector<std::pair<MapType::key_type, MapType::mapped_type>> include(section.begin(), section.end());
		std::sort(include.begin(), include.end(), [](const auto& l, const auto& r) { return l.second.line < r.second.line; });

        for (const auto& [key, value] : include) {
            // 将新的ini载入到本ini中，并判断文件编号防止死循环
			if (value.fileIndex == curFileIndex)
				this->load(basePath + "/" + value.value);
        }
    }
}

// 处理[]:[]
void IniFile::processInheritance(std::string& line, size_t endPos, int& lineNumber, std::string& curSectionName) {
	size_t colonPos = line.find(':', endPos + 1);
	if (colonPos != std::string::npos) {
		// 检查 ':' 之后的第一个字符是否是 '['
		if (colonPos + 1 >= line.size() || line[colonPos + 1] != '[') {
			WARNINGF(curSectionName, FileNames.size() - 1, lineNumber) << "继承格式不正确";
			return;
		}

		size_t nextEndPos = line.find(']', colonPos + 2);
		if (nextEndPos == std::string::npos) {
			ERRORF(curSectionName, FileNames.size() - 1, lineNumber) << "继承对象的中括号未闭合";
			return;
		}

		std::string inheritedName = line.substr(colonPos + 2, nextEndPos - colonPos - 2);
		if (!sections.count(inheritedName)) {
			ERRORF(curSectionName, FileNames.size() - 1, lineNumber) << "继承的节：\"" << inheritedName << "\"未找到";
			return;
		}

		auto& curSection = sections[curSectionName];
		auto& inheritedSection = sections[inheritedName];
		for (const auto& [key, value] : inheritedSection) {
			if (!curSection.count(key)) {
				Value inheritedValue = value;   // 复制原值
				inheritedValue.isInheritance = true; // 设置继承标志
				curSection[key] = inheritedValue; // 插入到当前节中
			}
			else if (curSection[key].isInheritance)
				WARNINGK(inheritedSection, key) << "重复的键，\"" << value << "\"被覆盖为\"" << inheritedSection[key] << "\".";
		}
	}
	else if (endPos != line.size() - 1) // 检查 ']' 是否是最后一个字符
		INFOF(curSectionName, FileNames.size() - 1, lineNumber) << "未以']'结尾";
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
