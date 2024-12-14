#pragma once
#include "Checker/LimitChecker.h"
#include "Checker/ListChecker.h"
#include "Checker/NumberChecker.h"
#include "Checker/TypeChecker.h"
#include "Dict.h"
#include "IniFile.h"
#include <string>
#include <unordered_map>

class Registry {
public:
	using Sections = std::unordered_map<std::string, Section>;
	operator std::string() const { return type; }

	Registry() = default;
	Registry(const Sections& config, const std::string& name, const Value& value);

	std::string type;
	// int defaultFile;
	bool checkExsit;
	std::vector<std::string> presetItems;
};

class Checker {
public:
	static Checker* Instance;
	static std::atomic<size_t> ProcessedSections;

	Checker(IniFile& configFile, IniFile& targetIni);
	void loadConfig(IniFile& configFile);
	void checkFile();

	void validate(const Section& section, const std::string& key, const Value& value, const std::string& type);
	
private:
	friend ListChecker;
	friend TypeChecker;
	template<class T>
	using map = std::unordered_map<std::string, T>;
	using Registrys = map<Registry>;
	using Globals = map<Dict>;
	using Sections = map<Dict>;
	using Limits = map<LimitChecker>;
	using Lists = map<ListChecker>;
	using Numbers = map<NumberChecker>;

	Registrys registries;	// 注册表名字映射: 配置ini的Type名字 <-> 注册ini中注册表名字(注册表可能不存在,则value="")
	Numbers numberLimits;	// 特殊类型限制: 类型名 <-> 特殊限制类型section
	Limits limits;			// 特殊类型限制: 类型名 <-> 特殊限制类型section
	Lists lists;			// 特殊类型限制: 类型名 <-> 特殊限制类型section
	Globals globals;		// 全局类型限制: 类型名 <-> 确定名字类型section
	Sections sections;		// 实例类型限制: 类型名 <-> 自定义类型section
	IniFile* targetIni;		// 检查的ini

	int validateInteger(const Section& section, const std::string& key, const Value& str);
	float validateFloat(const Section& section, const std::string& key, const Value& str);
	double validateDouble(const Section& section, const std::string& key, const Value& str);
	std::string validateString(const Section& section, const std::string& key, const Value& str);
};
