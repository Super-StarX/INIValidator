#include "Checker.h"
#include "Helper.h"
#include "Log.h"
#include "RegistryChecker.h"

RegistryChecker::RegistryChecker(Checker* checker, const Sections& config, const std::string& name, const Value& value)
	:checker(checker), type(value) {
	if (config.contains(name)) {
		const auto& registry = config.at(name);
		if (registry.contains("Type"))
			type = registry.at("Type");
		if (registry.contains("CheckExist"))
			checkExsit = string::isBool(registry.at("CheckExist"));
		if (registry.contains("PresetItems"))
			presetItems = string::split(registry.at("PresetItems"));
	}
}

void RegistryChecker::validateAllPreserItems(const Section::Key& registryName) const {
	for (const auto& item : presetItems) {
		validatePreserItem(registryName, item);
	}
}

void RegistryChecker::validatePreserItem(const Section::Key& registryName, const std::string& item) const {
	if (!checker->targetIni->sections.contains(item) && checkExsit) {
		Log::warning<_SectionExsit>({ registryName, 1, -1 }, item);
		return;
	}
	if (!checker->sections.contains(type)) {
		Log::print<_TypeNotExist>({ registryName, 1, -1 }, type);
		return;
	}

	checker->sections[type].validateSection(checker->targetIni->sections[item], type);
}

void RegistryChecker::validateSection(const Section::Key& registryName, const Value& name) const {
	if (!checker->targetIni->sections.contains(name) && checkExsit) {
		Log::warning<_SectionExsit>({ registryName, name.fileIndex, name.line }, name.value);
		return;
	}
	if (!checker->sections.contains(type)) {
		Log::print<_TypeNotExist>({ registryName, name.fileIndex, name.line }, type);
		return;
	}

	checker->sections[type].validateSection(checker->targetIni->sections[name.value], type);
}
