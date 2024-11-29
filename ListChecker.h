#pragma once
#include "IniFile.h"
#include "LimitChecker.h"
#include <string>
#include <unordered_map>

// 用于value列表的的值
// [Lists]
// Type = 列表的数据类型, 可填Sections、NumberLimits、Limits中的值
// Range = 列表长度最小值, 列表长度最大值
class Checker;
class ListChecker {
public:
	using Lists = std::unordered_map<std::string, ListChecker>;

	ListChecker() = default;
	ListChecker(Checker* checker, const Section& config);
	std::string validate(const Section& section, const std::string& key, const Value& value) const;
	
private:
	Checker* checker;
	std::string type; // 列表中元素的类型
	int minRange = 0;
	int maxRange = INT_MAX;
};
