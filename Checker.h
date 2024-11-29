#pragma once
#include "IniFile.h"
#include "ListChecker.h"
#include "LimitChecker.h"
#include "NumberChecker.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <set>

class Checker {
public:
    Checker(IniFile& configFile, IniFile& targetIni);
    void loadConfig(IniFile& configFile);
    void checkFile();
	void validate(const Section& section, const std::string& key, const Value& value, const std::string& type);

private:
	using Limits = std::unordered_map<std::string, LimitChecker>;
	using Lists = std::unordered_map<std::string, ListChecker>;
	using Numbers = std::unordered_map<std::string, NumberChecker>;
	
	Section registryMap;		// 注册表名字映射: 配置ini的Type名字 <-> 注册ini中注册表名字(注册表可能不存在,则value="")
	Numbers numberLimits;		// 特殊类型限制: 类型名 <-> 特殊限制类型section
	Limits limits;				// 特殊类型限制: 类型名 <-> 特殊限制类型section
	Lists lists;				// 特殊类型限制: 类型名 <-> 特殊限制类型section
	Sections sections;			// 常规类型限制: 类型名 <-> 常规限制类型section
	IniFile& targetIni;	// 检查的ini

    void validateSection(const std::string& sectionName, const Section& object, const std::string& type = "");

	std::string isInteger(const Value& str);
	std::string isFloat(const Value& str);
	std::string isDouble(const Value& str);
	std::string isString(const Value& str); 
	std::string limitCheck(const Value& str, const std::string& type);
};
