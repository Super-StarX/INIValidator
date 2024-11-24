#pragma once
#include "IniFile.h"
#include "LimitChecker.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <set>

class Checker {
public:
    Checker(const IniFile& configFile, const IniFile& targetIni);
    void loadConfig(const IniFile& configFile);
    void checkFile();

private:
	// 注册表名字映射: 配置ini的Type名字 <-> 注册ini中注册表名字(注册表可能不存在,则value="")
	KeyValues registryMap;
	// 特殊类型限制: 类型名 <-> 特殊限制类型section
    std::unordered_map<std::string, LimitChecker> limits;
	// 常规类型限制: 类型名 <-> 常规限制类型section
	std::unordered_map<std::string, KeyValues> sections;
	// 检查的ini
	const IniFile& targetIni;

    void validateSection(const std::string& sectionName, const KeyValues& object, const std::string& type = "");

    bool validate(const std::string& key, const Value& value, const std::string& type);
    bool isNumber(const std::string& str);
    bool isFloat(const std::string& str);
};
