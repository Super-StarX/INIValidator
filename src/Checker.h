#pragma once
#include "Checker/CustomChecker.h"
#include "Checker/LimitChecker.h"
#include "Checker/ListChecker.h"
#include "Checker/NumberChecker.h"
#include "Checker/RegistryChecker.h"
#include "Checker/TypeChecker.h"
#include "Dict.h"
#include "IniFile.h"
#include <string>
#include <unordered_map>

class Checker {
public:
	static Checker* Instance;
	static std::atomic<size_t> ProcessedSections;

	Checker(IniFile& configFile, IniFile& targetIni);
	void loadConfig(IniFile& configFile);
	void checkFile();

	void validate(const Section& section, const std::string& key, const Value& value, const std::string& type);
	
private:
	template<class T>
	using map = std::unordered_map<std::string, T>;
	using Registrys = map<RegistryChecker>;
	using Globals = map<Dict>;
	using Sections = map<Dict>;
	using Scripts = std::unique_ptr<CustomChecker>;
	using Limits = map<LimitChecker>;
	using Lists = map<ListChecker>;
	using Numbers = map<NumberChecker>;

	friend RegistryChecker;
	friend ListChecker;
	friend TypeChecker;

	Registrys registries;	// 注册表名字映射: 配置ini的Type名字 <-> 注册ini中注册表名字(注册表可能不存在,则value="")
	Numbers numberLimits;	// 特殊类型限制: 类型名 <-> 数字限制类型section
	Limits limits;			// 特殊类型限制: 类型名 <-> 范围限制类型section
	Lists lists;			// 特殊类型限制: 类型名 <-> 列表限制类型section
	Globals globals;		// 全局类型限制: 类型名 <-> 确定名字类型section
	Sections sections;		// 实例类型限制: 类型名 <-> 自定义类型section
	Scripts scripts;		// 实例类型限制: 类型名 <-> 自定义检查器
	IniFile* targetIni;		// 检查的ini

	int validateInteger(const Section& section, const std::string& key, const Value& str);
	float validateFloat(const Section& section, const std::string& key, const Value& str);
	double validateDouble(const Section& section, const std::string& key, const Value& str);
	std::string validateString(const Section& section, const std::string& key, const Value& str);
};
