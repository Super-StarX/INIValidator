#include "LimitChecker.h"
#include <sstream>

void LimitChecker::loadFromConfig(const std::unordered_map<std::string, std::string>& config) {
    if (config.count("StartWith")) {
        std::istringstream stream(config.at("StartWith"));
        std::string token;
        while (std::getline(stream, token, ',')) {
            startWith.push_back(token);
        }
    }
    if (config.count("EndWith")) {
        std::istringstream stream(config.at("EndWith"));
        std::string token;
        while (std::getline(stream, token, ',')) {
            endWith.push_back(token);
        }
    }
    if (config.count("LimitIn")) {
        std::istringstream stream(config.at("LimitIn"));
        std::string token;
        while (std::getline(stream, token, ',')) {
            limitIn.push_back(token);
        }
    }
    if (config.count("IgnoreCase")) {
        char res = config.at("IgnoreCase")[0];
        ignoreCase = res == '1' || res == 'y' || res == 't';
    }
}

bool LimitChecker::validate(const std::string& value) const {
    return matchesStart(value) && matchesEnd(value) && matchesList(value);
}

bool LimitChecker::matchesStart(const std::string& value) const {
    if (startWith.empty()) return true;
    std::string target = ignoreCase ? toLower(value) : value;
    for (const auto& prefix : startWith) {
        std::string checkPrefix = ignoreCase ? toLower(prefix) : prefix;
        if (target.rfind(checkPrefix, 0) == 0) { // 检查是否以 prefix 开头
            return true;
        }
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
        if (target == checkItem) {
            return true;
        }
    }
    return false;
}

std::string LimitChecker::toLower(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}
