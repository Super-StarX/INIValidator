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
			// 已经存进来的是继承来的, 那么本section的值就不存了, 因为按理说被继承来的覆盖了
			if (!oldValue.isInheritance) {
				// 如果是同文件内的覆盖, 就报警, 跨文件不报
				if (oldValue.fileIndex == fileIndex) {
					WARNINGK(section, key) << "重复的键，"
						<< oldValue.getFileName() << " 第" << oldValue.line << "行:\"" << oldValue
						<< "\"被覆盖为\"" << value << "\".";
				}
				section[key] = { value, lineNumber,fileIndex };
			}
		}
		else
			section[key] = { value, lineNumber,fileIndex };
    }
    else {
        // 仅有键, 无值, 用于配置ini的注册表, 暂时不报错, 未来会改
        section[line] = { "", lineNumber,fileIndex };
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
void IniFile::processInheritance(std::string& line, size_t endPos, int& lineNumber, std::string& curSectionName) {
	size_t colonPos = line.find(':', endPos + 1);
	if (colonPos != std::string::npos) {
		// 检查 ':' 之后的第一个字符是否是 '['
		if (colonPos + 1 < line.size() && line[colonPos + 1] == '[') {
			size_t nextEndPos = line.find(']', colonPos + 2);
			if (nextEndPos == std::string::npos) {
				ERRORF(curSectionName, FileNames.back(), lineNumber) << "继承对象的中括号未闭合";
				return;
			}

			std::string inheritedName = line.substr(colonPos + 2, nextEndPos - colonPos - 2);
			if (sections.count(inheritedName)) {
				auto& curSection = sections[curSectionName];
				auto& inheritedSection = sections[inheritedName];
				for (const auto& [key, value] : inheritedSection) {
					if (!curSection.count(key)) {
						Value inheritedValue = value;   // 复制原值
						inheritedValue.isInheritance = true; // 设置继承标志
						curSection[key] = inheritedValue; // 插入到当前节中
					}
					else if(curSection[key].isInheritance)
						WARNINGK(inheritedSection, key) << "重复的键，\"" << value << "\"被覆盖为\"" << inheritedSection[key] << "\".";
				}
			}
			else
				ERRORF(curSectionName, FileNames.back(), lineNumber) << "继承的节：\"" << inheritedName << "\"未找到";
		}
		else
			WARNINGF(curSectionName, FileNames.back(), lineNumber) << "继承格式不正确";
	}
	else if (endPos != line.size() - 1) // 检查 ']' 是否是最后一个字符
		INFOF(curSectionName, FileNames.back(), lineNumber) << "未以']'结尾";
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
