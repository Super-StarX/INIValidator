#pragma once
#include <string>
#include <unordered_map>

class Value {
public:
	std::string value;
	int Line;
	operator std::string() const { return value; }
	operator int() const { return Line; }
};
using KeyValues = std::unordered_map<std::string, Value>;
using Sections = std::unordered_map<std::string, KeyValues>;

class IniFile {
public:
    IniFile(const std::string& filepath);

    void load(const std::string& filepath);
    void readSection(std::string& line, int& lineNumber, std::string& currentSection);
    void readKeyValue(std::string& currentSection, std::string& line, int lineNumber);

    Sections sections;
private:
    void processIncludes(const std::string& basePath);
    void processInheritance();
    std::string removeInlineComment(const std::string& str);
    std::string trim(const std::string& str);
};
