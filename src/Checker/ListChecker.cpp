#include "Checker.h"
#include "Helper.h"
#include "ListChecker.h"
#include "Log.h"
#include <sstream>

ListChecker::ListChecker(Checker* checker, const Section& config) :checker(checker) {
	// 加载 Type
	if (!config.contains("Type")) {
		Log::error<_ListCheckerUnknownType>(config.line);
		return;
	}
	types = string::splitAsString(config.at("Type").value);

	// 加载 Range
	if (config.contains("Range")) {
		std::istringstream rangeStream(config.at("Range"));
		rangeStream >> minRange;
		if (rangeStream.peek() == ',')
			rangeStream.ignore();
		rangeStream >> maxRange;
		if (minRange > maxRange) {
			Log::error<_ListCheckerRangeIllegal>(config.line);
			return;
		}
	}
}

void ListChecker::validate(const Section& section, const std::string& key, const Value& value) const {
	int line = value.line;
	std::vector<Value> values;
	std::istringstream stream(value);
	std::string item;
	// 获取列表元素
	while (std::getline(stream, item, ','))
		values.push_back(Value(item, line));

	// 验证 Range
	if (values.size() < minRange || values.size() > maxRange) {
		Log::error<_ListCheckerOverRange>({ section,key }, minRange, maxRange);
		return;
	}

	// 验证每个元素
	for (const auto& element : values)
		for (const auto& type : types)
			checker->validate(section, key, element, type);
}