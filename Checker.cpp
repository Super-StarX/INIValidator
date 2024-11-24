#include "Checker.h"
#include "Log.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>
#include <set>

Checker::Checker(const IniFile& configFile, const IniFile& targetIni) :targetIni(targetIni) {
	loadConfig(configFile);
}

// 加载配置文件
void Checker::loadConfig(const IniFile& configFile) {
    // 加载 Limits
    if (configFile.sections.count("Limits"))
        for (const auto& [limitKey, _] : configFile.sections.at("Limits"))
            if (configFile.sections.count(limitKey))
                limits[limitKey] = LimitChecker(configFile.sections.at(limitKey));

    // 加载 Sections
    if (configFile.sections.count("Sections")) {
		registryMap = configFile.sections.at("Sections");
        for (const auto& [type, _] : registryMap) {
            if (configFile.sections.count(type)) {
                sections[type] = configFile.sections.at(type);
            }
        }
    }
}

// 验证每个注册表的内容
void Checker::checkFile() {
    for (const auto& [type, registryName] : registryMap) {
        if (!targetIni.sections.count(registryName)) {
            INFO(-1) << "Missing registry section: " << registryName;
            continue;
        }

		// 遍历目标ini的注册表的每个注册项
		auto& registry = targetIni.sections.at(registryName);
        for (const auto& [_, object] : registry) {
            const auto& [name, lineNumber] = object;
			if(!targetIni.sections.count(name)) {
				WARNING(lineNumber) << "Section \"" << name << "\" referenced in " << registryName << " is not implemented.";
				continue;
			}
            validateSection(name, targetIni.sections.at(name), type);
        }
    }
}

// 验证某个节
void Checker::validateSection(const std::string& sectionName, const KeyValues& object, const std::string& type) {
    if (!sections.count(type)) {
        LOG << "Unknown type \"" << type << "\" for section \"" << sectionName << "\".";
        return;
    }

    const auto& dict = sections.at(type);
    for (const auto& [key, value] : object) {
        if (!dict.count(key)) {
            INFO(value) << "Key \"" << key << "\" in section \"" << sectionName << "\" is not defined in the configuration.";
            continue;
        }

		auto& expectedType = dict.at(key).value;
        if (!validate(key, value, expectedType))
            ERROR(value) << "Invalid value for " << sectionName << "." << key << ": " << value << " (expected: " << expectedType << ")";
    }
}


// 验证键值对
bool Checker::validate(const std::string& key, const Value& value, const std::string& type) {
    if (type == "int") return isNumber(value);
    if (type == "float" || type == "double") return isFloat(value);
    if (type == "string") return value.value.size() < 256;
    if (limits.count(type)) return limits.at(type).validate(value);
	if (sections.count(type)) {
		if (targetIni.sections.count(value))
			validateSection(value, targetIni.sections.at(value), type);
		else
			ERROR(value.Line) << "Referenced section \"" << value << "\" of type \"" << type << "\" is not implemented.";
	}
    return true;
}

bool Checker::isNumber(const std::string& str) { return std::regex_match(str, std::regex("^-?\\d+$")); }
bool Checker::isFloat(const std::string& str) { return std::regex_match(str, std::regex("^-?\\d+(\\.\\d+)?$")); }