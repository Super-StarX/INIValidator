#include "Checker.h"
#include "Helper.h"
#include "Log.h"
#include "RegistryChecker.h"
#include "Settings.h"

RegistryChecker::RegistryChecker(Checker* checker, const Sections& config, const std::string& name, const Value& value)
	:checker(checker), type(value) {
	if (config.contains(name)) {
		const auto& registry = config.at(name);
		if (registry.contains("Type"))
			type = registry.at("Type");
		if (registry.contains("CheckExist"))
			checkExist = string::isBool(registry.at("CheckExist"));
		if (registry.contains("PresetItems"))
			presetItems = string::split(registry.at("PresetItems"));
		if (registry.contains("FileType"))
			fileType = registry.at("FileType");
		else
			fileType = Settings::Instance->defaultFile;
	}
}

void RegistryChecker::validateAllPreserItems(const Section::Key& registryName) const {
	for (const auto& item : presetItems)
		validatePreserItem(registryName, item);
}

void RegistryChecker::validatePreserItem(const Section::Key& registryName, const std::string& item) const {
	if (!checker->targetIni->sections.contains(item)) {
		if (checkExist)
			Log::warning<_SectionExist>({ registryName, 1, -1 }, item);
		return;
	}
	if (!checker->sections.contains(type)) {
		Log::print<_TypeNotExist>({ registryName, 1, -1 }, type);
		return;
	}
	
	checker->sections[type].validateSection(checker->targetIni->sections[item], type);
}

void RegistryChecker::validateSection(const Section::Key& registryName, const Value& name) const {
	if (!checker->targetIni->sections.contains(name)) {
		if (checkExist)
			Log::warning<_SectionExist>({ registryName, name.fileIndex, name.line }, name.value);
		return;
	}

	checker->sections[type].validateSection(checker->targetIni->sections[name.value], type);
}

bool RegistryChecker::hasPresetItems() const {
	return !presetItems.empty();
}
