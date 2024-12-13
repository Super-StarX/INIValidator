#pragma once
#include "IniFile.h"

class TypeChecker {
public:
	static void validate(const Section& section, const std::string& key, const Value& value, const std::string& type);
};

