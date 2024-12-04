#pragma once
#include "IniFile.h"
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

// 用于对字符串类型的特殊判定
// [Limits]
// StartWith = 前缀的限定内容, 不填则不检查
// EndWith = 后缀的限定内容, 不填则不检查
// LimitIn = 整体的限定内容, 不填则不检查
// IgnoreCase = 是否忽略大小写检查, 作用于前面三条
class LimitChecker {
public:
	LimitChecker(){};
	LimitChecker(const Section& config);
	std::vector<std::string> getToken(const Section& config, const std::string& key);
	void validate(const std::string& value) const;
	LimitChecker& operator=(const LimitChecker& other) {
		if (this == &other) return *this;
		startWith = other.startWith;
		endWith = other.endWith;
		limitIn = other.limitIn;
		caseSensitive = other.caseSensitive;
		return *this;
	}
private:
	std::string matchesStart(const std::string& value) const;
	std::string matchesEnd(const std::string& value) const;
	std::string matchesList(const std::string& value) const;
	std::string matchesLength(const std::string& value) const;
    std::string checkLower(const std::string& str) const;

	std::vector<std::string> startWith;
	std::vector<std::string> endWith;
	std::vector<std::string> limitIn;
	int maxLength;
	bool caseSensitive = false;
};