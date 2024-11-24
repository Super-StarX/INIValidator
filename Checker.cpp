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
        for (const auto& [type, registry] : registryMap) {
			if (!configFile.sections.count(type)) {
				LOG << registry.line << "Missing config registry section:" << registry;
				continue;
			}
            sections[type] = configFile.sections.at(type);
        }
    }
}

// 验证每个注册表的内容
void Checker::checkFile() {
    for (const auto& [type, registryName] : registryMap) {
		// 无注册表的项不检查
		if (registryName.value.empty())
			continue;

		// 检查注册表是否有使用
        if (!targetIni.sections.count(registryName)) {
            INFO(registryName) << "Missing registry section: " << registryName;
            continue;
        }

		// 遍历目标ini的注册表的每个注册项
		auto& registry = targetIni.sections.at(registryName);
        for (const auto& [_, name] : registry) {

			if(!targetIni.sections.count(name)) {
				WARNING(name) << "Section \"" << name << "\" referenced in " << registryName.value << " is not implemented.";
				continue;
			}
			if (!sections.count(type)) {
				LOG << "Unknown type \"" << type << "\" for section \"" << name << "\".";
				return;
			}
			auto& objectData = targetIni.sections.at(name);
            validateSection(name, objectData, type);
        }
    }
}

// 验证某个节
void Checker::validateSection(const std::string& sectionName, const KeyValues& object, const std::string& type) {
    const auto& dict = sections.at(type);
    for (const auto& [key, value] : object) {
        if (!dict.count(key)) {
			// LOG << "Key \"" << key << "\" in section \"" << sectionName << "\" is not defined in the configuration.";
            continue;
        }
		
		validate(key, value, dict.at(key));
    }
}


// 验证键值对
void Checker::validate(const std::string& key, const Value& value, const std::string& type) {
    if (type == "int") return isInteger(value);
    if (type == "float" || type == "double") return isFloat(value);
    if (type == "string") return isString(value);
    if (limits.count(type)) return limitCheck(value, type);
	if (sections.count(type)) {
		if (targetIni.sections.count(value))
			validateSection(value, targetIni.sections.at(value), type);
		else
			ERROR(value) << "Referenced section \"" << value << "\" of type \"" << type << "\" is not implemented.";
	}
}

void Checker::isInteger(const Value& value) {
	try {
		std::size_t pos;
		auto result = std::stoi(value, &pos);
		if (pos != value.value.size())
			ERROR(value) << "Illegal int type data:" << value;
	}
	catch (const std::invalid_argument& e) {
		ERROR(value) << e.what();
	}
	catch (const std::out_of_range& e) {
		ERROR(value) << e.what();
	}
}

void Checker::isFloat(const Value& value) {
	try {
		std::size_t pos;
		auto result = std::stof(value, &pos);
		if (pos != value.value.size())
			ERROR(value) << "Illegal float type data:" << value;
	}
	catch (const std::invalid_argument& e) {
		ERROR(value) << e.what();
	}
	catch (const std::out_of_range& e) {
		ERROR(value) << e.what();
	}
}

void Checker::isDouble(const Value& value) {
	try {
		std::size_t pos;
		auto result = std::stod(value, &pos);
		if (pos != value.value.size())
			ERROR(value) << "Illegal double type data:" << value;
	}
	catch (const std::invalid_argument& e) {
		ERROR(value) << e.what();
	}
	catch (const std::out_of_range& e) {
		ERROR(value) << e.what();
	}
}

void Checker::isString(const Value& value) {
	if (value.value.size() > 256)
		ERROR(value) << "To long value:" << value;
}

void Checker::limitCheck(const Value& value, const std::string& type) {
	if (limits.at(type).validate(value))
		ERROR(value) << "To long value:" << value;
}