#include "Helper.h"
#include "IniFile.h"
#include "Log.h"
#include "ProgressBar.h"
#include <algorithm>
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <regex>
#include <sstream>
#include <Windows.h>

std::vector<std::string> IniFile::FileNames;
size_t IniFile::FileIndex = ULLONG_MAX;

std::string IniFile::GetFileName(size_t index) {
	return FileNames.at(index);
}

size_t IniFile::GetFileIndex() {
	return FileNames.size() - 1;
}

IniFile::IniFile(const std::string& filepath, bool isConfig) :isConfig(isConfig) {
	load(filepath);
}

void IniFile::load(const std::string& filepath) {
	auto path = std::regex_replace(filepath, std::regex("^\"|\"$"), "");

	if (!std::filesystem::exists(path)) {
		std::cerr << "File not found: " << path << std::endl;
		return;
	}

	std::wifstream file(path);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << path << std::endl;
		return;
	}

	FileIndex++;
	auto curFileIndex = FileIndex;
	auto fileName = std::filesystem::path(path).filename().string();
	FileNames.push_back(fileName);
	std::string origin, currentSection;
	int lineNumber = 0;

	size_t totalLines = std::count(std::istreambuf_iterator<wchar_t>(file), std::istreambuf_iterator<wchar_t>(), '\n');
	file.clear();
	file.seekg(0);
	file.imbue(std::locale("zh_CN.UTF-8"));

	std::string name = "[" + std::to_string(curFileIndex) + "] " + fileName + " ";
	Progress::start(name, totalLines);

	auto fromWString = [](const std::wstring& wstr) {
		// Get required buffer size for conversion
		int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		std::string result(sizeNeeded - 1, 0); // -1 是为了去掉 null 终止符
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], sizeNeeded, nullptr, nullptr);

		return result;
	};

	std::wstring wline;
	// 逐行扫描加载ini
	while (std::getline(file, wline)) {
		origin = fromWString(wline);
		lineNumber++;
		// 移除注释之后的东西,并掐头去尾
		auto line = string::removeComment(origin);
		line = string::trim(line);
		if (line.empty()) continue;

		if (line.front() == '[')
			readSection(currentSection, line, origin, lineNumber);
		else if (!currentSection.empty())
			readKeyValue(currentSection, line, origin, lineNumber);
		Progress::update();
	}
	Progress::stop();
	processIncludes(std::filesystem::path(path).parent_path().string());
}

// 开头是[则为节名
void IniFile::readSection(std::string& currentSection, std::string& line, std::string origin, int& lineNumber) {
	size_t endPos = line.find(']');
	if (endPos == std::string::npos) {
		Log::error<_BracketClosed>({ origin, GetFileIndex(), lineNumber });
		return;
	}
	currentSection = line.substr(1, endPos - 1);
	sections[currentSection].name = currentSection;
	sections[currentSection].line = lineNumber;
	sections[currentSection].origin = origin;
	processInheritance(line, endPos, lineNumber, currentSection);
}

// 读取键值对
void IniFile::readKeyValue(std::string& currentSection, std::string& line, std::string origin, int lineNumber) {
	auto& section = sections[currentSection];
	auto delimiterPos = line.find('=');
	if (delimiterPos != std::string::npos) {
		// 传统键值对
		auto key = string::trim(line.substr(0, delimiterPos));
		auto value = string::trim(line.substr(delimiterPos + 1));
		// += 的特殊处理
		if (key == "+") {
			static int var_num = 0;
			key = "var_" + std::to_string(var_num);
			++var_num;
		}
		else if (section.contains(key)) {
			auto& oldValue = section[key];
			// 如果现存的值是继承来的值，则不报警，新值覆盖后会去掉继承标签
			// 如果是同文件内的覆盖, 就报警, 跨文件不报
			if (!oldValue.isInheritance && oldValue.fileIndex == FileIndex)
				Log::error<_DuplicateKey>({ section.origin, oldValue.fileIndex, lineNumber },
					key, oldValue.line, oldValue, value);
		}
		section[key] = { value, lineNumber, origin, FileIndex };
	}
	else
		// 仅有键, 无值, 用于配置ini的注册表, 暂时不报错, 未来会改
		section[line] = { "", lineNumber, origin, FileIndex };
}

// 处理#include
void IniFile::processIncludes(const std::string& basePath) {
	// 找到名为#include的节
	if (sections.contains("#include")) {
		// 遍历#include里的所有键值对，因为unordered_map没有顺序，所以重新按顺序遍历
		size_t curFileIndex = FileIndex;
		auto& section = sections["#include"];
		using MapType = decltype(section.section);
		std::vector<std::pair<MapType::key_type, MapType::mapped_type>> include(section.begin(), section.end());
		std::sort(include.begin(), include.end(), [](const auto& l, const auto& r) { return l.second.line < r.second.line; });

		for (const auto& [key, value] : include)
			// 将新的ini载入到本ini中，并判断文件编号防止死循环
			if (value.fileIndex == curFileIndex)
				this->load(basePath + "/" + value.value);
	}
}

// 处理[]:[]
void IniFile::processInheritance(std::string& line, size_t endPos, int& lineNumber, std::string& curSectionName) {
	size_t colonPos = line.find(':', endPos + 1);
	if (colonPos != std::string::npos) {
		// 检查 ':' 之后的第一个字符是否是 '['
		if (colonPos + 1 >= line.size() || line[colonPos + 1] != '[')
			return Log::error<_SectionFormat>({ line, GetFileIndex(), lineNumber });

		size_t nextEndPos = line.find(']', colonPos + 2);
		if (nextEndPos == std::string::npos)
			return Log::error<_InheritanceBracketClosed>({ line, GetFileIndex(), lineNumber });

		std::string inheritedName = line.substr(colonPos + 2, nextEndPos - colonPos - 2);
		if (!sections.contains(inheritedName))
			return Log::error<_InheritanceSectionExsit>({ line, GetFileIndex(), lineNumber }, inheritedName);

		auto& curSection = sections[curSectionName];
		auto& inheritedSection = sections[inheritedName];
		for (const auto& [key, value] : inheritedSection) {
			if (!curSection.contains(key)) {
				Value inheritedValue = value;   // 复制原值
				inheritedValue.isInheritance = true; // 设置继承标志
				curSection[key] = inheritedValue; // 插入到当前节中
			}
			else if (curSection[key].isInheritance)
				Log::error<_InheritanceDuplicateKey>({ inheritedSection, key }, value, inheritedSection[key]);
		}
	}
	else if (endPos != line.size() - 1) // 检查 ']' 是否是最后一个字符
		Log::error<_InheritanceBracketClosed>({ line, GetFileIndex(), lineNumber });
}

std::string Value::getFileName() const {
	return IniFile::FileNames.at(fileIndex);
}
