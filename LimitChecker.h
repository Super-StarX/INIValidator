#pragma once
#include "IniFile.h"
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

class LimitChecker {
public:
	LimitChecker(){};
	LimitChecker(const Section& config);
	void getToken(const Section& config, const std::string& key, std::vector<std::string>& vec);
	std::string validate(const std::string& value) const;
	LimitChecker& operator=(const LimitChecker& other) {
		if (this == &other) return *this;
		startWith = other.startWith;
		endWith = other.endWith;
		limitIn = other.limitIn;
		ignoreCase = other.ignoreCase;
		return *this;
	}
private:
	std::string matchesStart(const std::string& value) const;
	std::string matchesEnd(const std::string& value) const;
	std::string matchesList(const std::string& value) const;
    std::string checkLower(const std::string& str) const;

	std::vector<std::string> startWith;
	std::vector<std::string> endWith;
	std::vector<std::string> limitIn;
	bool ignoreCase = false;
};