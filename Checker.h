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
	using Limits = std::unordered_map<std::string, LimitChecker>;
	
	Section registryMap;		// 注册表名字映射: 配置ini的Type名字 <-> 注册ini中注册表名字(注册表可能不存在,则value="")
	Limits limits;				// 特殊类型限制: 类型名 <-> 特殊限制类型section
	Sections sections;			// 常规类型限制: 类型名 <-> 常规限制类型section
	const IniFile& targetIni;	// 检查的ini

    void validateSection(const std::string& sectionName, const Section& object, const std::string& type = "");

	void validate(const std::string& key, const Value& value, const std::string& type);
	void isInteger(const Value& str);
	void isFloat(const Value& str);
	void isDouble(const Value& str);
	void isString(const Value& str);
	void limitCheck(const Value& str, const std::string& type);
};
