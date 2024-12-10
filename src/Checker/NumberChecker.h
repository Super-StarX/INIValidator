#pragma once
#include "IniFile.h"
#include <string>

// 需要限制上下限的数值类型
// [NumberLimits]
// Range = 数值下限, 数值上限
class NumberChecker {
public:
	NumberChecker() = default;
	NumberChecker(const Section& config);

	// 检查数字是否在指定范围内
	void validate(const std::string& value) const;

private:
	std::string type; // 类型，如 int, short, double
	float minRange { 0.f }; // 最小值
	float maxRange { 0.f }; // 最大值

	bool checkRange(float value) const;
};
