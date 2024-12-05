#pragma once
#include "IniFile.h"

class Checker;
class TypeChecker {
public:
	TypeChecker(Checker*checker);
	void validate(const Value& value, const std::string& type) const;
private:
	Checker* checker;
};

