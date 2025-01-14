#pragma once
#include "IniFile.h"
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
	using Set = std::unordered_set<std::string>;
	explicit Dict() = default;
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
	void validateSection(const Section& object, const std::string& type);
	void validate(const Section::Key& key, const Section::Key& vkey, const Section& object, const Value& value);

private:
	std::vector<std::string> dynamicKeys;			// 存储所有需要动态生成的key
	Map section;									// key <-> 该key对应的自定义类型
	Set keys;										// 存储该字典所有的键，用来检测键是否存在

	static DictData parseTypeValue(const std::string& str);
	static std::vector<std::string> generateKey(const std::string& dynamicKey, const Section& object);
	static double evaluateExpression(const std::string& expr, const Section& object);
	static double parseValue(size_t& i, const std::string& expr, const Section& object);
	static void applyOperation(std::stack<double>& values, std::stack<char>& operators);
};