#include "Settings.h"
#include "Helper.h"

Settings* Settings::Instance = nullptr;

Settings::Settings(const IniFile& configFile) {
	load(configFile);
	Instance = this;
}

void Settings::load(const IniFile& configFile) {
	if (configFile.sections.contains("INIValidator")) {
		const auto& section = configFile.sections.at("INIValidator");
		if (section.contains("FolderPath"))
			folderPath = section.at("FolderPath");
	}

	if (configFile.sections.contains("Files")) {
		const auto& filesSection = configFile.sections.at("Files");
		if (!filesSection.section.empty())
			defaultFile = filesSection.begin()->first;
		for (const auto& [file, keywords] : filesSection)
			files[file] = string::split(keywords);
	}
	
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
		READ(sections, SectionExist)
		READ(sections, UnreachableSection)

		READ(sections, BracketClosed)
		READ(sections, DuplicateKey)
		READ(sections, SectionFormat)
		READ(sections, InheritanceFormat)
		READ(sections, InheritanceBracketClosed)
		READ(sections, InheritanceSectionExist)
		READ(sections, InheritanceDuplicateKey)

		READ(sections, SpaceExistBetweenEqualSign)
		READ(sections, SpaceLostBetweenEqualSign)

		READ(sections, EmptyValue)
		READ(sections, IllegalValue)
		READ(sections, OverlongValue)
		READ(sections, IntIllegal)
		READ(sections, FloatIllegal)
		READ(sections, OverlongString)
		
		READ(sections, TypeCheckerTypeNotExist)

		READ(sections, NumberCheckerOverRange)

		READ(sections, LimitCheckerPrefixIllegal)
		READ(sections, LimitCheckerSuffixIllegal)
		READ(sections, LimitCheckerValueIllegal)
		READ(sections, LimitCheckerOverRange)
		READ(sections, ListCheckerUnknownType)
		READ(sections, ListCheckerRangeIllegal)
		READ(sections, ListCheckerOverRange)

		#undef READ
	}
}