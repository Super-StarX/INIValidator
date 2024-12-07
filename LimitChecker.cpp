#include "Helper.h"
#include "LimitChecker.h"
#include <sstream>

LimitChecker::LimitChecker(const Section& config) {
	startWith = getToken(config, "StartWith");
	endWith = getToken(config, "EndWith");
	limitIn = getToken(config, "LimitIn");
	if (config.contains("MaxLength"))
		maxLength = std::stoi(config.at("MaxLength"));
	if (config.contains("CaseSenstive")) {
		char res = config.at("CaseSenstive").value[0];
		caseSensitive = res == '1' || res == 'y' || res == 't';
	}
}

std::vector<std::string> LimitChecker::getToken(const Section& config, const std::string& key) {
	if (!config.contains(key))
		return std::vector<std::string>();
	return string::split(config.at(key).value);
}

void LimitChecker::validate(const std::string& value) const {
	auto result = matchesStart(value) + matchesEnd(value) + matchesList(value);
	if (!result.empty())
		throw result;
}

std::string LimitChecker::matchesStart(const std::string& value) const {
    if (startWith.empty()) return std::string();
    auto target = checkLower(value);
    for (const auto& prefix : startWith) {
		auto checkPrefix = checkLower(prefix);
        if (target.rfind(checkPrefix, 0) == 0) // 检查是否以 prefix 开头
            return std::string();
    }
	return value + "前缀不符合规则";
}

std::string LimitChecker::matchesEnd(const std::string& value) const {
    if (endWith.empty()) return std::string();
	auto target = checkLower(value);
    for (const auto& suffix : endWith) {
		auto checkSuffix = checkLower(suffix);
        if (target.size() >= checkSuffix.size() &&
            target.compare(target.size() - checkSuffix.size(), checkSuffix.size(), checkSuffix) == 0) {
            return std::string();
        }
    }
    return value + "后缀不符合规则";
}

std::string LimitChecker::matchesList(const std::string& value) const {
    if (limitIn.empty()) return std::string();
	auto target = checkLower(value);
    for (const auto& item : limitIn) {
		auto checkItem = checkLower(item);
        if (target == checkItem)
            return std::string();
    }
	return value + "不属于限定范围内的值";
}

std::string LimitChecker::matchesLength(const std::string& value) const {
	return value.length() > maxLength ? 
		value + "长度超过最大值: 当前(" + std::to_string(value.length()) + ") > 最大(" + std::to_string(maxLength) + ")" :
		std::string();
}

std::string LimitChecker::checkLower(const std::string& str) const {
	if (caseSensitive)
		return str;
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}
