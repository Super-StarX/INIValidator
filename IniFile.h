#pragma once
#include <string>
#include <unordered_map>

class IniFile {
public:
    IniFile(const std::string& filepath);
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sections;

    void load(const std::string& filepath);

private:
    void processIncludes(const std::string& basePath);
    void processInheritance();
    std::string trim(const std::string& str);
    std::string removeInlineComment(const std::string& str);
};
