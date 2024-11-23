#pragma once
#include "IniFile.h"
#include "LimitChecker.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <set>

class Checker {
public:
    Checker(const IniFile& configFile);
    void loadConfig(const IniFile& configFile);
    void checkFile(const IniFile& targetIni);

private:
    std::unordered_map<std::string, LimitChecker> limits; // 限制规则
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sections; // Sections内容

    void handleInheritance(std::unordered_map<std::string, std::string>& section);
    void parseRegistry(const std::string& registrySection, const IniFile& targetIni, std::unordered_map<std::string, std::string>& registry);
    void validateSection(const std::string& sectionName, const std::unordered_map<std::string, std::string>& keys);

    bool validate(const std::string& key, const std::string& value, const std::string& type);
    bool isNumber(const std::string& str);
    bool isFloat(const std::string& str);
};
