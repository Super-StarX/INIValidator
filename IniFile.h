#pragma once
#include <string>
#include <unordered_map>

class Value {
public:
	operator std::string() const { return value; }

	std::string value { };
	int Line { -1 };
};
using KeyValues = std::unordered_map<std::string, Value>;
using Sections = std::unordered_map<std::string, KeyValues>;

class IniFile {
public:
	static std::string argv0;
    IniFile(const std::string& filepath, bool isConfig);

    void load(const std::string& filepath);
    void readSection(std::string& line, int& lineNumber, std::string& currentSection);
    void readKeyValue(std::string& currentSection, std::string& line, int lineNumber);

	bool isConfig { false };
    Sections sections;
private:
    void processIncludes(const std::string& basePath);
    void processInheritance();
    std::string removeInlineComment(const std::string& str);
    std::string trim(const std::string& str);
};
