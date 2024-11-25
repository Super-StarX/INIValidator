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
				LOG << registry.line << "缺少配置注册节：" << registry;
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
            INFOL(-1) << "没有注册表：" << registryName;
            continue;
        }

		// 遍历目标ini的注册表的每个注册项
		auto& registry = targetIni.sections.at(registryName);
        for (const auto& [_, name] : registry) {

			if(!targetIni.sections.count(name)) {
				WARNINGL(name.line) << "注册表\"" << registryName.value << "\"中声明的 " << name << " 未被实现";
				continue;
			}
			if (!sections.count(type)) {
				LOG << "\"" << name << "\"的类型\"" << type << "\"未知";
				return;
			}
			auto& objectData = targetIni.sections.at(name);
            validateSection(name, objectData, type);
        }
    }
}

// 验证某个节
void Checker::validateSection(const std::string& sectionName, const Section& object, const std::string& type) {
    const auto& dict = sections.at(type);
    for (const auto& [key, value] : object) {
        if (!dict.count(key)) {
			// LOG << "Key \"" << key << "\" in section \"" << sectionName << "\" is not defined in the configuration.";
            continue;
        }
		
		validate(object, key, value, dict.at(key));
    }
}


// 验证键值对
void Checker::validate(const Section& section, const std::string& key, const Value& value, const std::string& type) {
	std::string result;
	if (type == "int") result = isInteger(value);
    else if (type == "float" || type == "double") result = isFloat(value);
    else if (type == "string") result = isString(value);
    else if (limits.count(type)) result = limitCheck(value, type);
	else if (sections.count(type)) {
		if (targetIni.sections.count(value))
			validateSection(value, targetIni.sections.at(value), type);
		else
			ERRORL(value.line) << "\"" << type << "\"中声明的\"" << value << "\"未被实现";
	}

	if (!result.empty())
		ERRORK(section, key) << result;
}

std::string Checker::isInteger(const Value& value) {
	try {
		std::size_t pos;
		auto result = std::stoi(value, &pos);
		if (pos != value.value.size())
			return std::format("{}不是整数，非整数部分会被忽略", value.value);
	}
	catch (const std::invalid_argument) {
		return std::format("{}不是整数，结果为0", value.value);
	}
	catch (const std::out_of_range& e) {
		return e.what();
	}

	return std::string();
}

std::string Checker::isFloat(const Value& value) {
	try {
		std::size_t pos;
		auto result = std::stof(value, &pos);
		if (pos != value.value.size())
			return std::format("{}不是浮点数，非浮点数部分会被忽略", value.value);
	}
	catch (const std::invalid_argument) {
		return std::format("{}不是浮点数，会造成非预期的结果", value.value);
	}
	catch (const std::out_of_range& e) {
		return e.what();
	}

	return std::string();
}

std::string Checker::isDouble(const Value& value) {
	try {
		std::size_t pos;
		auto result = std::stod(value, &pos);
		if (pos != value.value.size())
			return std::format("{}不是浮点数，非浮点数部分会被忽略", value.value);
	}
	catch (const std::invalid_argument) {
		return std::format("{}不是浮点数，会造成非预期的结果", value.value);
	}
	catch (const std::out_of_range& e) {
		return e.what();
	}

	return std::string();
}

std::string Checker::isString(const Value& value) {
	if (value.value.size() > 256)
		return std::format("太长了：{}", value.value);

	return std::string();
}

std::string Checker::limitCheck(const Value& value, const std::string& type) {
	if (limits.at(type).validate(value))
		return std::format("太长了：{}", value.value);

	return std::string();
}