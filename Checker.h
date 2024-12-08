#pragma once
#include "IniFile.h"
#include "LimitChecker.h"
#include "ListChecker.h"
#include "NumberChecker.h"
#include "TypeChecker.h"
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

class DictData {
public:
	std::vector<std::string> types;
	std::string defaultValue;
	std::string file;
};

class Dict {
public:
	using Map = std::unordered_map<std::string, DictData>;
	Dict() {};
	Dict(const Section& config);
	auto begin() { return section.begin(); }
	auto begin() const { return section.begin(); }
	auto end() { return section.end(); }
	auto end() const { return section.end(); }
	void insert(const Map& other) { return section.insert(other.begin(), other.end()); }
	bool contains(const std::string& key) const { return section.contains(key); }
	DictData at(const std::string& key) const { return section.at(key); }
	DictData& at(const std::string& key) { return section.at(key); }
	DictData& operator[](const std::string& key) { return section[key]; }
	void validateSection(const Section& object, const std::string& type = "");

	std::vector<std::string> dynamicKeys;			// 存储所有需要动态生成的key
	Map section;									// key <-> 该key对应的自定义类型
private:
	DictData parseTypeValue(const std::string& str);
};

class Checker {
public:
	static Checker* Instance;

	Checker(IniFile& configFile, IniFile& targetIni);
	void loadConfig(IniFile& configFile);
	void checkFile();

	void validate(const Section& section, const std::string& key, const Value& value, const std::string& type);
	std::vector<std::string> generateKey(const std::string& dynamicKey, const Section& object) const;

private:
	friend ListChecker;
	friend TypeChecker;
	using Globals = std::unordered_map<std::string, Dict>;
	using Sections = std::unordered_map<std::string, Dict>;
	using Limits = std::unordered_map<std::string, LimitChecker>;
	using Lists = std::unordered_map<std::string, ListChecker>;
	using Numbers = std::unordered_map<std::string, NumberChecker>;

	Section registryMap;	// 注册表名字映射: 配置ini的Type名字 <-> 注册ini中注册表名字(注册表可能不存在,则value="")
	Numbers numberLimits;	// 特殊类型限制: 类型名 <-> 特殊限制类型section
	Limits limits;			// 特殊类型限制: 类型名 <-> 特殊限制类型section
	Lists lists;			// 特殊类型限制: 类型名 <-> 特殊限制类型section
	Globals globals;		// 全局类型限制: 类型名 <-> 确定名字类型section
	Sections sections;		// 实例类型限制: 类型名 <-> 自定义类型section
	IniFile* targetIni;		// 检查的ini

	double evaluateExpression(const std::string& expr, const Section& object) const;
	double parseValue(size_t& i, const std::string& expr, const Section& object) const;
	void applyOperation(std::stack<double>& values, std::stack<char>& operators) const;
	int validateInteger(const Section& section, const std::string& key, const Value& str);
	float validateFloat(const Section& section, const std::string& key, const Value& str);
	double validateDouble(const Section& section, const std::string& key, const Value& str);
	std::string validateString(const Section& section, const std::string& key, const Value& str);
};
