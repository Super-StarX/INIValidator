#include "ListChecker.h"
#include "Checker.h"
#include "Log.h"
#include <sstream>

ListChecker::ListChecker(Checker* checker, const Section& config) :checker(checker) {
	// 加载 Type
	if (!config.count("Type"))
		throw std::runtime_error("ListChecker 配置缺少 Type");
	type = config.at("Type").value;

	// 加载 Range
	if (config.count("Range")) {
		std::istringstream rangeStream(config.at("Range"));
		rangeStream >> minRange;
		if (rangeStream.peek() == ',')
			rangeStream.ignore();
		rangeStream >> maxRange;
		if (minRange > maxRange)
			throw std::runtime_error("Range 配置错误：最小值大于最大值");
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
	if (values.size() < minRange || values.size() > maxRange)
		throw key + " 列表项数超出范围，应在 [" + std::to_string(minRange) + "," + std::to_string(maxRange) + "] 内";

	// 验证每个元素
	for (const auto& element : values)
		checker->validate(section, key, element, type);
}