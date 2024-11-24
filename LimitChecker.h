#pragma once
#include "IniFile.h"
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

class LimitChecker {
public:
	LimitChecker(){};
	LimitChecker(const KeyValues& config);
	void getToken(const KeyValues& config, const std::string& key, std::vector<std::string>& vec);
    bool validate(const std::string& value) const;

private:
    bool matchesStart(const std::string& value) const;
    bool matchesEnd(const std::string& value) const;
    bool matchesList(const std::string& value) const;
    std::string toLower(const std::string& str) const;

	std::vector<std::string> startWith;
	std::vector<std::string> endWith;
	std::vector<std::string> limitIn;
	bool ignoreCase = false;
};