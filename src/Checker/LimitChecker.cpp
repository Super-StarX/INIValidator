#include "Helper.h"
#include "LimitChecker.h"
#include "Log.h"
#include <sstream>

LimitChecker::LimitChecker(const Section& config) {
	startWith = getToken(config, "StartWith");
	endWith = getToken(config, "EndWith");
	limitIn = getToken(config, "LimitIn");
	for (const auto& [key, _] : config) {
		if (key.rfind("LimitIn.", 0) == 0) {
			auto parts = getToken(config, key);
			limitIn.insert(limitIn.end(), parts.begin(), parts.end());
		}
	}

	if (config.contains("MaxLength"))
		maxLength = std::stoi(config.at("MaxLength"));
	if (config.contains("CaseSenstive")) {
		char res = config.at("CaseSenstive")().front();
		caseSensitive = res == '1' || res == 'y' || res == 't';
	}
}

std::vector<std::string> LimitChecker::getToken(const Section& config, const std::string& key) {
	if (!config.contains(key))
		return std::vector<std::string>();
	return string::split(config.at(key).value);
}

void LimitChecker::validate(const Section& section, const std::string& key, const std::string& value) const {
	if (matchesStart(section, key, value))
		if (matchesEnd(section, key, value))
			if (matchesList(section, key, value))
				matchesLength(section, key, value);
}

bool LimitChecker::matchesStart(const Section& section, const std::string& key, const std::string& value) const {
    if (startWith.empty()) return true;
    auto target = checkLower(value);
    for (const auto& prefix : startWith) {
		auto checkPrefix = checkLower(prefix);
        if (target.rfind(checkPrefix, 0) == 0) // 检查是否以 prefix 开头
            return true;
    }

	Log::error<_LimitCheckerPrefixIllegal>({ section,key }, value);
	return false;
}

bool LimitChecker::matchesEnd(const Section& section, const std::string& key, const std::string& value) const {
    if (endWith.empty()) return true;
	auto target = checkLower(value);
    for (const auto& suffix : endWith) {
		auto checkSuffix = checkLower(suffix);
        if (target.size() >= checkSuffix.size() &&
            target.compare(target.size() - checkSuffix.size(), checkSuffix.size(), checkSuffix) == 0) {
            return true;
        }
    }

	Log::error<_LimitCheckerSuffixIllegal>({ section,key }, value);
	return false;
}

bool LimitChecker::matchesList(const Section& section, const std::string& key, const std::string& value) const {
    if (limitIn.empty()) return true;
	auto target = checkLower(value);
    for (const auto& item : limitIn) {
		auto checkItem = checkLower(item);
        if (target == checkItem)
            return true;
    }

	Log::error<_LimitCheckerValueIllegal>({ section,key }, value);
	return false;
}

void LimitChecker::matchesLength(const Section& section, const std::string& key, const std::string& value) const {
	if (value.length() > maxLength)
		Log::error<_LimitCheckerOverRange>({ section,key }, value.length(), maxLength);
}

std::string LimitChecker::checkLower(const std::string& str) const {
	if (caseSensitive)
		return str;
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}
