#pragma once
#include "IniFile.h"
#include <string>
#include <unordered_map>
#include <vector>

class Checker;
class RegistryChecker {
public:
	using Sections = std::unordered_map<std::string, Section>;
	operator std::string() const { return type; }

	RegistryChecker() = default;
	RegistryChecker(Checker* checker, const Sections& config, const std::string& name, const Value& value);
	void validateAllPreserItems(const Section::Key& registryName) const;
	void validatePreserItem(const Section::Key& registryName, const std::string& item) const;
	void validateSection(const Section::Key& registryName, const Value& name) const;
	bool hasPresetItems() const;

private:
	Checker* checker{ nullptr };
	std::string type{ };
	std::string fileType{ };
	// int defaultFile;
	bool checkExist{ };
	std::vector<std::string> presetItems{ };
};
