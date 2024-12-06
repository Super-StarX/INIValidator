#include "Settings.h"
#include "Helper.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

Settings::Settings(const IniFile& configFile) {
	load(configFile);
}

#define READ(sections, variable)	\
    if (sections.count(#variable))	\
        variable = string::isBool(sections.at(#variable));

void Settings::load(const IniFile& configFile) {
	if (configFile.sections.count("LogSetting")) {
		auto& sections = configFile.sections.at("LogSetting");
		READ(sections, RecordKeyNotExist)
		READ(sections, RecordTypeNotExist)
		READ(sections, RecordDynamicKeyVariableError)
		READ(sections, RecordDynamicKeyFormatError)
		READ(sections, CheckUnusedGlobal)
		READ(sections, CheckUnusedRegistry)
		READ(sections, CheckSectionExsit)
		READ(sections, CheckBracketClosed)
		READ(sections, CheckDuplicateKey)
		READ(sections, CheckSectionFormat)
		READ(sections, CheckInheritanceFormat)
		READ(sections, CheckInheritanceSectionExsit)
		READ(sections, CheckSpaceExistBeforeEqualSign)
		READ(sections, CheckSpaceExistAfterEqualSign)
		READ(sections, CheckEmptyValue)
	}
}
#undef READ