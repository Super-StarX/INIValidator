#pragma once
#include "IniFile.h"
#include "LimitChecker.h"
#include <string>
#include <unordered_map>

class Checker;
class ListChecker {
public:
	ListChecker() = default;
	ListChecker(const Section& config, const std::unordered_map<std::string, LimitChecker>& limits, IniFile& targetIni);
	std::string validate(Checker* section, const std::string& key, const Value& value) const;
	
private:
	std::string type; // 列表中元素的类型
	int minRange = 0;
	int maxRange = INT_MAX;
};
