#include "Checker.h"
#include "Helper.h"
#include "Log.h"
#include "ProgressBar.h"
#include <iostream>
#include <set>
#include <sstream>
#include <windows.h>

Checker* Checker::Instance = nullptr;
std::atomic<size_t> Checker::ProcessedSections(0);

Checker::Checker(IniFile& configFile, IniFile& targetIni) : targetIni(&targetIni) {
	loadConfig(configFile);
	Instance = this;
}

// 加载配置文件
void Checker::loadConfig(IniFile& configFile) {
	// 加载字符串限制器
	if (configFile.sections.contains("Limits"))
		for (const auto& [key, _] : configFile.sections.at("Limits"))
			if (configFile.sections.contains(key))
				limits[key] = LimitChecker(configFile.sections.at(key));

	// 加载列表限制器
	if (configFile.sections.contains("Lists"))
		for (const auto& [key, _] : configFile.sections.at("Lists"))
			if (configFile.sections.contains(key))
				lists[key] = ListChecker(this, configFile.sections.at(key));

	// 加载数值限制器
	if (configFile.sections.contains("NumberLimits"))
		for (const auto& [key, _] : configFile.sections.at("NumberLimits"))
			if (configFile.sections.contains(key))
				numberLimits[key] = NumberChecker(configFile.sections.at(key));

	// 加载注册表
	if (configFile.sections.contains("Registries"))
		registryMap = configFile.sections.at("Registries");

	// 加载全局类型
	if (configFile.sections.contains("Globals"))
		for (const auto& [key, _] : configFile.sections.at("Globals"))
			if (configFile.sections.contains(key))
				globals[key] = Dict(configFile.sections.at(key));

	// 加载实例类型
	if (configFile.sections.contains("Sections"))
		for (const auto& [key, _] : configFile.sections.at("Sections"))
			if (configFile.sections.contains(key))
				sections[key] = Dict(configFile.sections.at(key));
}

// 验证每个注册表的内容
void Checker::checkFile() {
	ProgressBar::INIFileProgress.stop();
	ProgressBar::CheckerProgress.addProgressBar(0, "Checker", targetIni->sections.size());

	// [Globals] General
	for (const auto& [sectionName, _] : globals) {
		if (!targetIni->sections.contains(sectionName)) {
			Log::info<_UnusedGlobal>(-1, sectionName);
			continue;
		}
		globals[sectionName].validateSection(targetIni->sections[sectionName], sectionName);
	}

	// [Registries] VehicleTypes=UnitType
	for (const auto& [registryName, type] : registryMap) {
		// 检查注册表是否有使用
		if (!targetIni->sections.contains(registryName)) {
			Log::info<_UnusedRegistry>(-1, registryName);
			continue;
		}

		// 遍历目标ini的注册表的每个注册项
		auto& registry = targetIni->sections.at(registryName);
		for (const auto& [_, name] : registry) {
			if (!targetIni->sections.contains(name)) {
				Log::warning<_SectionExsit>({ registryName, name.fileIndex, name.line }, name.value);
				continue;
			}
			if (!sections.contains(type)) {
				Log::print<_TypeNotExist>({ registryName, name.fileIndex, name.line }, type.value);
				return;
			}

			sections[type].validateSection(targetIni->sections[name.value], type.value);
		}
	}

	for (const auto& [name, section] : targetIni->sections) {
		if (!section.isScanned) {
			//std::this_thread::sleep_for(std::chrono::microseconds(1));
			Log::info<_UnreachableSection>({ section.headLine }, section.name);
			ProgressBar::CheckerProgress.updateProgress(0, ++Checker::ProcessedSections);
		}
	}

	ProgressBar::CheckerProgress.stop();
}

// 验证键值对
void Checker::validate(const Section& section, const std::string& key, const Value& value, const std::string& type) {
	if (value.value.empty())
		return Log::error<_EmptyValue>({ section, key }, key);

	if (type == "int") validateInteger(section, key, value);
	else if (type == "float") validateFloat(section, key, value);
	else if (type == "double") validateDouble(section, key, value);
	else if (type == "string") validateString(section, key, value);
	else if (numberLimits.contains(type)) numberLimits.at(type).validate(value.value);
	else if (limits.contains(type)) limits.at(type).validate(section, key, value);
	else if (lists.contains(type)) lists.at(type).validate(section, key, value); // 新增
	else if (sections.contains(type)) {
		if (value.value == "none" || value.value == "<none>")
			return;

		if (!targetIni->sections.contains(value)) {
			if (type != "AnimType")
				Log::error< _TypeCheckerTypeNotExist>({ section,key }, type, value);
		}
		else
			sections.at(type).validateSection(targetIni->sections.at(value), type);
	}
}

int Checker::validateInteger(const Section& section, const std::string& key, const Value& value) {
	int result = 0;
	try {
		int base = 10;
		std::string buffer = value;
		if (buffer.front() == '$') {
			buffer = buffer.substr(1, buffer.size());
			base = 16;
		}
		else if (tolower(buffer.back()) == 'h') {
			buffer = buffer.substr(0, buffer.size() - 1);
			base = 16;
		}

		std::size_t pos;
		result = std::stoi(buffer, &pos);
		if (pos != buffer.size())
			Log::error<_IntIllegal>({ section, key }, value);
	}
	catch (const std::invalid_argument) {
		Log::error<_IllegalValue>({ section, key }, value);
	}
	catch (const std::out_of_range) {
		Log::error<_OverlongValue>({ section, key }, value);
	}
	return result;
}

float Checker::validateFloat(const Section& section, const std::string& key, const Value& value) {
	float result = 0.0f;
	try {
		std::string buffer = value;
		if (buffer.back() == '%')
			buffer = buffer.substr(0, buffer.size() - 1);

		if (buffer.front() == '.')
			buffer = "0" + buffer;

		std::size_t pos;
		result = std::stof(buffer, &pos);
		if (pos != buffer.size())
			Log::error<_FloatIllegal>({ section,key }, value);

		if (value.value.back() == '%')
			result /= 100;
	}
	catch (const std::invalid_argument) {
		Log::error<_IllegalValue>({ section, key }, value);
	}
	catch (const std::out_of_range) {
		Log::error<_OverlongValue>({ section, key }, value);
	}
	return result;
}

double Checker::validateDouble(const Section& section, const std::string& key, const Value& value) {
	double result = 0.0;
	try {
		std::string buffer = value;
		if (buffer.back() == '%')
			buffer = buffer.substr(0, buffer.size() - 1);

		std::size_t pos;
		result = std::stod(buffer, &pos);
		if (pos != buffer.size())
			Log::error<_FloatIllegal>({ section,key }, value);

		if (value.value.back() == '%')
			result /= 100;
	}
	catch (const std::invalid_argument) {
		Log::error<_IllegalValue>({ section, key }, value);
	}
	catch (const std::out_of_range) {
		Log::error<_OverlongValue>({ section, key }, value);
	}
	return result;
}

std::string Checker::validateString(const Section& section, const std::string& key, const Value& value) {
	if (value.value.size() > 512)
		Log::error<_OverlongString>({ section, key }, value);

	return value;
}