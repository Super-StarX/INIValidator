#include "NumberChecker.h"

NumberChecker::NumberChecker(const Section& config) {
	if (config.count("Range")) {
		std::string rangeStr = config.at("Range").value;
		size_t commaPos = rangeStr.find(',');
		if (commaPos != std::string::npos) {
			minRange = std::stof(rangeStr.substr(0, commaPos));
			maxRange = std::stof(rangeStr.substr(commaPos + 1));
		}
	}

	if (config.count("Type")) {
		type = config.at("Type").value;
	}
}

void NumberChecker::validate(const std::string& value) const {
	float intValue = std::stof(value);
	if (!checkRange(intValue))
		throw "值 " + value + " 不在范围 [" + std::to_string(minRange) + ", " + std::to_string(maxRange) + "] 内";
}

bool NumberChecker::checkRange(int value) const {
	return value >= minRange && value <= maxRange;
}