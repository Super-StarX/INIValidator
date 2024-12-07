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
    if (sections.contains(#variable))	\
        variable = sections.at(#variable);

void Settings::load(const IniFile& configFile) {
	if (configFile.sections.contains("LogSetting")) {
		auto& sections = configFile.sections.at("LogSetting");
		READ(sections, recordKeyNotExist)
		READ(sections, recordTypeNotExist)
		READ(sections, recordDynamicKeyVariableError)
		READ(sections, recordDynamicKeyFormatError)
		READ(sections, checkUnusedGlobal)
		READ(sections, checkUnusedRegistry)
		READ(sections, checkSectionExsit)
		READ(sections, checkBracketClosed)
		READ(sections, checkDuplicateKey)
		READ(sections, checkSectionFormat)
		READ(sections, checkInheritanceFormat)
		READ(sections, checkInheritanceSectionExsit)
		READ(sections, checkSpaceExistBeforeEqualSign)
		READ(sections, checkSpaceExistAfterEqualSign)
		READ(sections, checkEmptyValue)
	}
}
#undef READ