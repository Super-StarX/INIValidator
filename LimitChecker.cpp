#include "LimitChecker.h"
#include <sstream>

LimitChecker::LimitChecker(const Section& config) {
	getToken(config, "StartWith", startWith);
	getToken(config, "EndWith", endWith);
	getToken(config, "LimitIn", limitIn);
	if (config.count("IgnoreCase")) {
		// 似乎有专有名词"Case-insensitive"
		char res = config.at("IgnoreCase").value[0];
		ignoreCase = res == '1' || res == 'y' || res == 't';
	}
}

void LimitChecker::getToken(const Section& config, const std::string& key, std::vector<std::string>& vec) {
	if (!config.count(key))
		return;
	std::istringstream stream(config.at(key).value);
	std::string token;
	while (std::getline(stream, token, ','))
		vec.push_back(token);
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

std::string LimitChecker::checkLower(const std::string& str) const {
	if (!ignoreCase)
		return str;
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}
