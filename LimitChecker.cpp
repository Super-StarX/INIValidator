#include "LimitChecker.h"
#include <sstream>

LimitChecker::LimitChecker(const KeyValues& config) {
	getToken(config, "StartWith", startWith);
	getToken(config, "EndWith", endWith);
	getToken(config, "LimitIn", limitIn);
	if (config.count("IgnoreCase")) {
		char res = config.at("IgnoreCase").value[0];
		ignoreCase = res == '1' || res == 'y' || res == 't';
	}
}

void LimitChecker::getToken(const KeyValues& config, const std::string& key, std::vector<std::string>& vec) {
	if (!config.count(key))
		return;
	std::istringstream stream(config.at(key).value);
	std::string token;
	while (std::getline(stream, token, ','))
		vec.push_back(token);
}

bool LimitChecker::validate(const std::string& value) const {
    return matchesStart(value) && matchesEnd(value) && matchesList(value);
}

bool LimitChecker::matchesStart(const std::string& value) const {
    if (startWith.empty()) return true;
    std::string target = ignoreCase ? toLower(value) : value;
    for (const auto& prefix : startWith) {
        std::string checkPrefix = ignoreCase ? toLower(prefix) : prefix;
        if (target.rfind(checkPrefix, 0) == 0) // 检查是否以 prefix 开头
            return true;
    }
    return false;
}

bool LimitChecker::matchesEnd(const std::string& value) const {
    if (endWith.empty()) return true;
    std::string target = ignoreCase ? toLower(value) : value;
    for (const auto& suffix : endWith) {
        std::string checkSuffix = ignoreCase ? toLower(suffix) : suffix;
        if (target.size() >= checkSuffix.size() &&
            target.compare(target.size() - checkSuffix.size(), checkSuffix.size(), checkSuffix) == 0) {
            return true;
        }
    }
    return false;
}

bool LimitChecker::matchesList(const std::string& value) const {
    if (limitIn.empty()) return true;
    std::string target = ignoreCase ? toLower(value) : value;
    for (const auto& item : limitIn) {
        std::string checkItem = ignoreCase ? toLower(item) : item;
        if (target == checkItem)
            return true;
    }
    return false;
}

std::string LimitChecker::toLower(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}
