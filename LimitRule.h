#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

class LimitRule {
public:
    std::vector<std::string> startWith;
    std::vector<std::string> endWith;
    std::vector<std::string> limitIn;
    bool ignoreCase = false;

    void loadFromConfig(const std::unordered_map<std::string, std::string>& config);
    bool validate(const std::string& value) const;

private:
    bool matchesStart(const std::string& value) const;
    bool matchesEnd(const std::string& value) const;
    bool matchesList(const std::string& value) const;
    std::string toLower(const std::string& str) const;
};