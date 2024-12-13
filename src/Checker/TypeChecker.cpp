#include "Checker.h"
#include "Helper.h"
#include "Log.h"
#include "TypeChecker.h"

void TypeChecker::validate(const Section& section, const std::string& key, const Value& value, const std::string& type) {
	auto checker = Checker::Instance;
	if (value.value == "none" || value.value == "<none>")
		return;

	if (!checker->targetIni->sections.contains(value)) {
		if (type != "AnimType")
			Log::error< _TypeCheckerTypeNotExist>({ section,key }, type, value);
	}
	else
		checker->sections.at(type).validateSection(checker->targetIni->sections.at(value), type);
}