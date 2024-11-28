#include "ListChecker.h"
#include "Checker.h"
#include <sstream>

ListChecker::ListChecker(const Section& config, const std::unordered_map<std::string, LimitChecker>& limits, IniFile& targetIni)
	: limits(limits), targetIni(targetIni) {
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

std::string ListChecker::validate(const Section& section, const std::string& key, const Value& value) const {
	int line = value.line;
	std::vector<Value> elements;
	std::istringstream stream(value);
	std::string item;
	while (std::getline(stream, item, ','))
		elements.push_back(Value(item, line));

	// 验证 Range
	if (elements.size() < minRange || elements.size() > maxRange)
		return key + " 列表项数超出范围，应在 [" + std::to_string(minRange) + "," + std::to_string(maxRange) + "] 内";

	// 验证每个元素
	for (const auto& element : elements) {
		std::string result;
		if (type == "int") result = Checker::isInteger(element);
		else if (type == "float" || type == "double") result = Checker::isFloat(element);
		else if (type == "string") result = Checker::isString(element);
		else if (limits.count(type)) result = limits.at(type).validate(element);
		else if (section.count(type)) {
			if (targetIni.sections.count(value))
				validateSection(value, targetIni.sections.at(value), type);
			else
				ERRORL(value.line) << "\"" << type << "\"中声明的\"" << value << "\"未被实现";
		}
		else
			result = "未知的类型 \"" + type + "\"";

		if (!result.empty())
			return key + " 中的值 \"" + element + "\" 无效: " + result;
	}

	return std::string();
}