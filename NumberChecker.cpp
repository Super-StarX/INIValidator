#include "NumberChecker.h"

NumberChecker::NumberChecker(const Section& config) {
	if (config.count("Range")) {
		std::string rangeStr = config.at("Range").value;
		size_t commaPos = rangeStr.find(',');
		if (commaPos != std::string::npos) {
			minRange = std::stoi(rangeStr.substr(0, commaPos));
			maxRange = std::stoi(rangeStr.substr(commaPos + 1));
		}
	}

	if (config.count("Type")) {
		type = config.at("Type").value;
	}
}

std::string NumberChecker::validate(const std::string& value) const {
	try {
		int intValue = std::stoi(value); // 假设数值类型是整数
		if (checkRange(intValue)) {
			return std::string();
		}
		return "值 " + value + " 不在范围 [" + std::to_string(minRange) + ", " + std::to_string(maxRange) + "] 内";
	}
	catch (const std::invalid_argument&) {
		return "无法转换值 " + value + " 为整数";
	}
	catch (const std::out_of_range&) {
		return "值 " + value + " 超出整数范围";
	}
}

bool NumberChecker::checkRange(int value) const {
	return value >= minRange && value <= maxRange;
}