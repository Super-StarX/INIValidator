#include "Helper.h"
#include "Settings.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

Settings* Settings::Instance = nullptr;

Settings::Settings(const IniFile& configFile) {
	load(configFile);
}

void Settings::load(const IniFile& configFile) {
	if (configFile.sections.contains("LogSetting")) {
		auto& sections = configFile.sections.at("LogSetting");

		#define READ(sections, variable)	\
			if (sections.contains(#variable))	\
				variable = sections.at(#variable);

		READ(sections, KeyNotExist)
		READ(sections, TypeNotExist)
		READ(sections, DynamicKeyVariableError)
		READ(sections, DynamicKeyFormatError)
		READ(sections, UnusedGlobal)
		READ(sections, UnusedRegistry)
		READ(sections, SectionExsit)
		READ(sections, BracketClosed)
		READ(sections, DuplicateKey)
		READ(sections, SectionFormat)
		READ(sections, InheritanceFormat)
		READ(sections, InheritanceSectionExsit)
		READ(sections, SpaceExistBeforeEqualSign)
		READ(sections, SpaceExistAfterEqualSign)
		READ(sections, EmptyValue)
		#undef READ
	}

}
