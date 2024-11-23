#include "Checker.h"
#include "Log.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>
#include <set>

Checker::Checker(const IniFile& configFile) {
    loadConfig(configFile);
}

// 加载配置文件
void Checker::loadConfig(const IniFile& configFile) {
    // 加载 Limits
    if (configFile.sections.count("Limits")) {
        for (const auto& [limitKey, _] : configFile.sections.at("Limits")) {
            if (configFile.sections.count(limitKey)) {
                LimitChecker rule;
                rule.loadFromConfig(configFile.sections.at(limitKey));
                limits[limitKey] = rule;
            }
        }
    }

    // 加载 Sections
    if (configFile.sections.count("Sections")) {
        for (const auto& [sectionName, _] : configFile.sections.at("Sections")) {
            if (configFile.sections.count(sectionName)) {
                sections[sectionName] = configFile.sections.at(sectionName);
                handleInheritance(sections[sectionName]);
            }
        }
    }
}

// 处理继承关系 (支持 [A]:[B] 语法)
void Checker::handleInheritance(std::unordered_map<std::string, std::string>& section) {
    for (const auto& [key, value] : section) {
        if (key.back() == ':') {
            std::string parentSection = key.substr(0, key.size() - 1);
            if (sections.count(parentSection)) {
                section.insert(sections[parentSection].begin(), sections[parentSection].end());
            }
        }
    }
}

// 解析目标 INI 的注册表
void Checker::parseRegistry(const std::string& registrySection, const IniFile& targetIni, std::unordered_map<std::string, std::string>& registry) {
    if (targetIni.sections.count(registrySection)) {
        for (const auto& [keyStr, value] : targetIni.sections.at(registrySection)) {
            std::string key;
            if (keyStr == "+") {
                static int var_num = 0;
                key = "var_"+ std::to_string(var_num);
                ++var_num;
            }
            else if (registry.count(key))
                ERROR(__LINE__) << "Error: Duplicate key \"" << key << "\" in " << registrySection << ", value \"" << registry[key] << "\" overwritten by \"" << value << "\".";
            registry[key] = value;
        }
    }
}

// 验证每个注册节的内容
void Checker::checkFile(const IniFile& targetIni) {
    for (const auto& [registryName, _] : sections) {
        std::unordered_map<std::string, std::string> registry;
        parseRegistry(registryName, targetIni, registry);

        for (const auto& [_, sectionName] : registry) {
            if (targetIni.sections.count(sectionName))
                validateSection(sectionName, targetIni.sections.at(sectionName));
            else
                ERROR(__LINE__) << "Error: Section \"" << sectionName << "\" referenced in " << registryName << " not found.";
        }
    }
}

// 验证某个节
void Checker::validateSection(const std::string& sectionName, const std::unordered_map<std::string, std::string>& keys) {
    if (!sections.count(sectionName)) {
        ERROR(__LINE__) << "Warning: No configuration found for section \"" << sectionName << "\".";
        return;
    }

    const auto& rules = sections[sectionName];
    for (const auto& [key, value] : keys) {
        std::string type = rules.count(key) ? rules.at(key) : "";
        if (!validate(key, value, type))
            ERROR(__LINE__) << "Error: Invalid value for " << sectionName << "." << key << ": " << value;
    }
}

// 验证键值对
bool Checker::validate(const std::string& key, const std::string& value, const std::string& type) {
    if (type == "int") return isNumber(value);
    if (type == "float" || type == "double") return isFloat(value);
    if (type == "string") return value.size() < 256;

    // 检查是否为特殊类型
    if (limits.count(type)) {
        return limits.at(type).validate(value);
    }
    return true;
}

bool Checker::isNumber(const std::string& str) { return std::regex_match(str, std::regex("^-?\\d+$")); }
bool Checker::isFloat(const std::string& str) { return std::regex_match(str, std::regex("^-?\\d+(\\.\\d+)?$")); }