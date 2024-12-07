#include "Helper.h"
#include "Settings.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

Settings::Settings(const IniFile& configFile) {
	load(configFile);
}


void Settings::load(const IniFile& configFile) {
	if (configFile.sections.contains("LogSetting")) {
		auto& sections = configFile.sections.at("LogSetting");

		#define READ(sections, variable)	\
			if (sections.contains(#variable))	\
				variable = sections.at(#variable);

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
		#undef READ
	}

}
