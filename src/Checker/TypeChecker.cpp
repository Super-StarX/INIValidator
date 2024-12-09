#include "Checker.h"
#include "Helper.h"
#include "Log.h"
#include "TypeChecker.h"

TypeChecker::TypeChecker(Checker* checker):checker(checker) {
}

void TypeChecker::validate(const Value& value, const std::string& type) const {
	if (!checker->targetIni->sections.contains(value))
		throw std::string("\"" + type + "\"中声明的\"" + value + "\"未被实现");
	checker->sections.at(type).validateSection(checker->targetIni->sections.at(value), type);
}