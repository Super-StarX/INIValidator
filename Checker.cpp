#include "Checker.h"
#include "Log.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>
#include <set>

Checker::Checker(IniFile& configFile, IniFile& targetIni) :targetIni(targetIni) {
	loadConfig(configFile);
}

// 加载配置文件
void Checker::loadConfig(IniFile& configFile) {
    // 加载 Limits
    if (configFile.sections.count("Limits"))
        for (const auto& [limitKey, _] : configFile.sections.at("Limits"))
            if (configFile.sections.count(limitKey))
                limits[limitKey] = LimitChecker(configFile.sections.at(limitKey));

	// 加载 Lists
	if (configFile.sections.count("Lists"))
		for (const auto& [listKey, _] : configFile.sections.at("Lists"))
			if (configFile.sections.count(listKey))
				lists[listKey] = ListChecker(this, configFile.sections.at(listKey));

	// 加载 NumberLimits
	if (configFile.sections.count("NumberLimits"))
		for (const auto& [numberKey, _] : configFile.sections.at("NumberLimits"))
			if (configFile.sections.count(numberKey))
				numberLimits[numberKey] = NumberChecker(configFile.sections.at(numberKey));

    // 加载 Sections
    if (configFile.sections.count("Sections")) {
		registryMap = configFile.sections.at("Sections");
        for (const auto& [type, registry] : registryMap) {
			if (!configFile.sections.count(type)) {
				INFOK(registryMap, registry) << "缺少配置注册节：" << registry;
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
				WARNINGF(registryName.value, name.fileIndex, name.line) << "该注册表声明的 " << name << " 未被实现";
				continue;
			}
			if (!sections.count(type)) {
				LOG << "\"" << name << "\"的类型\"" << type << "\"未知";
				return;
			}
			Section& objectData = targetIni.sections[name.value];
			if (!objectData.isScanned) {
				objectData.isScanned = true;
				validateSection(name, objectData, type);
			}
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
	try {
		if (type == "int") isInteger(value);
		else if (type == "float") isFloat(value);
		else if (type == "double") isDouble(value);
		else if (type == "string") isString(value);
		else if (numberLimits.count(type)) numberLimits.at(type).validate(value.value);
		else if (limits.count(type)) limits.at(type).validate(value);
		else if (lists.count(type)) lists.at(type).validate(section, key, value); // 新增
		else if (sections.count(type)) {
			if (targetIni.sections.count(value))
				validateSection(value, targetIni.sections.at(value), type);
			else
				ERRORL(value.line) << "\"" << type << "\"中声明的\"" << value << "\"未被实现";
		}
	}
	catch (const std::string& e) {
		ERRORK(section, key) << e;
	}
	catch (const std::invalid_argument) {
		ERRORK(section, key) << value << "是非法参数";
	}
	catch (const std::out_of_range) {
		ERRORK(section, key) << value << "超过限制";
	}
	catch (...) {
		ERRORK(section, key) << "非预期的错误";
	}
}

int Checker::isInteger(const Value& value) {
	int base = 10;
	std::string buffer = value;
	if (*value.value.begin() == '$') {
		buffer = buffer.substr(1, buffer.size());
		base = 16;
	}
	else if (tolower(value.value.back()) == 'h') {
		buffer = buffer.substr(0, buffer.size() - 1);
		base = 16;
	}

	std::size_t pos;
	auto result = std::stoi(value, &pos);
	if (pos != value.value.size())
		throw value + "不是整数，非整数部分会被忽略";

	return result;
}

float Checker::isFloat(const Value& value) {
	std::string buffer = value;
	if (buffer.back() == '%')
		buffer = buffer.substr(0, buffer.size() - 1);

	std::size_t pos;
	auto result = std::stof(value, &pos);
	if (pos != value.value.size())
		throw value + "不是浮点数，非浮点数部分会被忽略";

	if (value.value.back() == '%')
		result /= 100;

	return result;
}

double Checker::isDouble(const Value& value) {
	std::string buffer = value;
	if (buffer.back() == '%')
		buffer = buffer.substr(0, buffer.size() - 1);

	std::size_t pos;
	auto result = std::stod(value, &pos);
	if (pos != value.value.size())
		throw value + "不是浮点数，非浮点数部分会被忽略";

	if (value.value.back() == '%')
		result /= 100;

	return result;
}

std::string Checker::isString(const Value& value) {
	if (value.value.size() > 512)
		throw "值超过最大字数限制：" + value;

	return value;
}

void Checker::limitCheck(const Value& value, const std::string& type) {
	return limits.at(type).validate(value);
}