#pragma once
#include <string>
#include <unordered_map>

static std::vector<std::string> FileNames;
class Value {
public:
	operator std::string() const { return value; }
	std::string getFileName() const { return FileNames.at(fileIndex); }

	std::string value { };
	int line { -1 };
	int fileIndex { -1 };
	bool isInheritance { false };
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
	void processInheritance(std::string& line, size_t endPos, int& lineNumber, std::string& currentSection);
    std::string removeInlineComment(const std::string& str);
    std::string trim(const std::string& str);
};
