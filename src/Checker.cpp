#include "Checker.h"
#include "Helper.h"
#include "ProgressBar.h"
#include "Log.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <regex>
#include <set>
#include <sstream>

Checker* Checker::Instance = nullptr;
std::atomic<size_t> processedSections(0);

Dict::Dict(const Section& config) {
	for (const auto& [key, value] : config) {
		if (key.find('(') != std::string::npos && key.find(')') != std::string::npos)
			dynamicKeys.push_back(key);

		section[key] = parseTypeValue(value);
	}
}

void Dict::validateSection(const Section& object, const std::string& type) {
	if (object.isScanned) return;
	++processedSections;
	ProgressBar::CheckerProgress.updateProgress(0, processedSections);
	const_cast<Section&>(object).isScanned = true;
	auto pChecker = Checker::Instance;

	for (const auto& dynamicKey : this->dynamicKeys) {
		try {
			auto keys = pChecker->generateKey(dynamicKey, object);
			for (const auto& key : keys)
				if (object.contains(key))
					for (const auto& type : this->at(dynamicKey).types)
						pChecker->validate(object, key, object.at(key), type);
		}
		catch (const std::string& e) {
			Log::warning<_DynamicKeyVariableError>(object.section.begin()->second.line, e);
		}
		catch (const std::invalid_argument) {
			// 不做任何处理
		}
	}

	for (const auto& [key, value] : object) {
		if (!this->contains(key))
			Log::print<_KeyNotExist>({ object, key }, key);
			continue;

		for (const auto& type : this->at(key).types)
			pChecker->validate(object, key, value, type);
	}
}

DictData Dict::parseTypeValue(const std::string& str) {
	DictData retval;
	std::istringstream ss(str);
	std::string valueStr;
	std::getline(ss, valueStr, ',');
	std::getline(ss, retval.defaultValue, ',');
	std::getline(ss, retval.file, ',');
	retval.types = string::splitAsString(valueStr);

	return retval;
}

Checker::Checker(IniFile& configFile, IniFile& targetIni) : targetIni(&targetIni) {
	loadConfig(configFile);
	Instance = this;
	ProgressBar::INIFileProgress.stop();
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

	ProgressBar::CheckerProgress.stop();
}

// 用于生成动态key
std::vector<std::string> Checker::generateKey(const std::string& dynamicKey, const Section& object) const {
	std::vector<std::string> generatedKeys;

	// 解析特殊格式 Stage(0,WeaponStage)
	size_t startPos = dynamicKey.find('(');
	size_t endPos = dynamicKey.find(')');

	if (startPos != std::string::npos && endPos != std::string::npos && endPos > startPos) {
		auto insideBrackets = dynamicKey.substr(startPos + 1, endPos - startPos - 1);  // 获取括号内的内容
		auto parts = string::split(insideBrackets);  // 按逗号分割
		if (parts.size() == 2) {
			// 解析起始值和终止值
			double startValue = evaluateExpression(parts[0], object);
			double endValue = evaluateExpression(parts[1], object);

			// 确保生成的范围是整数
			int start = static_cast<int>(startValue);
			int end = static_cast<int>(endValue);

			// 生成从 start 到 end 的所有 key
			for (int i = start; i <= end; ++i) {
				auto generatedKey = dynamicKey.substr(0, startPos) + std::to_string(i) + dynamicKey.substr(endPos + 1);
				generatedKeys.push_back(generatedKey);  // 添加生成的 key
			}
		}
		else
			throw std::string("动态键格式错误: " + dynamicKey);
	}

	return generatedKeys;
}

// 用于从表达式中获取数字或引用
double Checker::evaluateExpression(const std::string& expr, const Section& object) const {
	if (string::isNumber(expr))
		return std::stod(expr); // 如果是数字，直接返回

	// 不是表达式, 尝试直接进行替换
	if (!string::isExpression(expr))
		if (object.contains(expr))
			return std::stod(object.at(expr).value);

	std::stack<double> values;
	std::stack<char> operators;

	for (size_t i = 0; i < expr.length(); ++i) {
		char c = expr[i];

		// 如果是数字或变量名
		if (std::isdigit(c) || std::isalpha(c))
			values.push(parseValue(i, expr, object));
		// 如果是左括号，入栈
		else if (c == '(')
			operators.push(c);
		// 如果是右括号，计算括号内的表达式
		else if (c == ')') {
			while (!operators.empty() && operators.top() != '(')
				applyOperation(values, operators);
			if (operators.empty() || operators.top() != '(')
				throw std::string("动态键表达式中的括号不匹配: " + expr);
			operators.pop(); // 弹出 '('
		}
		// 如果是运算符
		else if (c == '+' || c == '-' || c == '*' || c == '/') {
			while (!operators.empty() && math::precedence(operators.top()) >= math::precedence(c))
				applyOperation(values, operators);
			operators.push(c);
		}
		else
			throw std::string("动态键表达式中的字符无效: " + std::string(1, c));
	}

	// 处理栈中剩余的操作符
	while (!operators.empty())
		applyOperation(values, operators);

	if (values.size() != 1)
		throw std::string("动态键表达式无效: " + expr);

	return values.top();
}

double Checker::parseValue(size_t& i, const std::string& expr, const Section& object) const {
	std::string value;
	while (i < expr.length() && (std::isalnum(expr[i]) || expr[i] == '.'))
		value += expr[i++];
	--i;

	// 是数字, 直接返回具体值
	if (string::isNumber(value))
		return std::stod(value);

	// 不是数字, 看看是否是变量
	if (!object.contains(value))
		throw std::invalid_argument("动态键中存在未定义的变量: " + value);

	// 是否是数字型变量
	value = object.at(value).value;
	if (!string::isNumber(value))
		throw std::string("动态键中存在非数字变量: " + value);

	return std::stod(value);
}

void Checker::applyOperation(std::stack<double>& values, std::stack<char>& operators) const {
	double b = values.top(); values.pop();
	double a = values.top(); values.pop();
	char op = operators.top(); operators.pop();
	values.push(math::applyOperation(a, b, op));
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
	else if (limits.contains(type)) limits.at(type).validate(value);
	else if (lists.contains(type)) lists.at(type).validate(section, key, value); // 新增
	else if (sections.contains(type)) {
		if (value.value == "none" || value.value == "<none>")
			return;

		if (!targetIni->sections.contains(value)) {
			if (type != "AnimType")
				throw std::string("\"" + type + "\"中声明的\"" + value + "\"未被实现");
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
			throw std::string(value + "不是浮点数，非浮点数部分会被忽略");

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
			throw std::string(value + "不是浮点数，非浮点数部分会被忽略");

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