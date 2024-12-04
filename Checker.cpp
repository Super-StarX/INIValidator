#include "Checker.h"
#include "Log.h"
#include "Helper.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>
#include <set>

Checker::Checker(IniFile& configFile, IniFile& targetIni) :targetIni(&targetIni) {
	loadConfig(configFile);
}

// 加载配置文件
void Checker::loadConfig(IniFile& configFile) {
    // 加载 Limits
    if (configFile.sections.count("Limits"))
        for (const auto& [limitKey, _] : configFile.sections.at("Limits"))
            if (configFile.sections.count(limitKey))
                limits[limitKey] = LimitChecker(configFile.sections.at(limitKey));

	// 加载 Lists
	if (configFile.sections.count("Lists"))
		for (const auto& [listKey, _] : configFile.sections.at("Lists"))
			if (configFile.sections.count(listKey))
				lists[listKey] = ListChecker(this, configFile.sections.at(listKey));

	// 加载 NumberLimits
	if (configFile.sections.count("NumberLimits"))
		for (const auto& [numberKey, _] : configFile.sections.at("NumberLimits"))
			if (configFile.sections.count(numberKey))
				numberLimits[numberKey] = NumberChecker(configFile.sections.at(numberKey));

    // 加载 Sections
    if (configFile.sections.count("Sections")) {
		registryMap = configFile.sections.at("Sections");
        for (const auto& [type, registry] : registryMap) {
			if (!configFile.sections.count(type)) {
				INFOK(registryMap, registry) << "缺少配置注册节：" << registry;
				continue;
			}
			// 检查所有 key，处理 dynamicKeys 和直接生成的 key
			auto& targetSection = sections[type];
			for (const auto& [key, value] : configFile.sections.at(type)) {
				if (key.find('(') != std::string::npos && key.find(')') != std::string::npos)
					targetSection.dynamicKeys.push_back(key);
				
				targetSection.section[key] = value;
			}
        }
    }
}

// 验证每个注册表的内容
void Checker::checkFile() {
    for (const auto& [type, registryName] : registryMap) {
		// 无注册表的项不检查
		if (registryName.value.empty())
			continue;

		// 检查注册表是否有使用
        if (!targetIni->sections.count(registryName)) {
            INFOL(-1) << "没有注册表：" << registryName;
            continue;
        }

		// 遍历目标ini的注册表的每个注册项
		auto& registry = targetIni->sections.at(registryName);
        for (const auto& [_, name] : registry) {
			if(!targetIni->sections.count(name)) {
				WARNINGF(registryName.value, name.fileIndex, name.line) << "该注册表声明的 " << name << " 未被实现";
				continue;
			}
			if (!sections.count(type)) {
				LOG << "\"" << name << "\"的类型\"" << type << "\"未知";
				return;
			}

			validateSection(name, targetIni->sections[name.value], type);
        }
    }
}

// 验证某个节
void Checker::validateSection(const std::string& sectionName, const Section& object, const std::string& type) {
	if (object.isScanned) return;
	const_cast<Section&>(object).isScanned = true;

    const auto& dict = sections.at(type);
	for (const auto& dynamicKey : dict.dynamicKeys) {
		try {
			auto keys = generateKey(dynamicKey, object);
			for (const auto& key : keys)
				if (object.count(key))
					validate(object, key, object.at(key), dict.at(dynamicKey));
		}
		catch (const std::string& e) {
			WARNINGL(object.section.begin()->second.line) << e;
		}
		catch (const std::invalid_argument) {
			// 不做任何处理
		}
	}

    for (const auto& [key, value] : object) {
        if (!dict.count(key))
			// LOG << "Key \"" << key << "\" in section \"" << sectionName << "\" is not defined in the configuration.";
            continue;
		
		validate(object, key, value, dict.at(key));
    }
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
		if (object.count(expr))
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
	if (!object.count(value))
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
	try {
		if (value.value.empty())
			throw std::string("未填写值");

		if (type == "int") isInteger(value);
		else if (type == "float") isFloat(value);
		else if (type == "double") isDouble(value);
		else if (type == "string") isString(value);
		else if (numberLimits.count(type)) numberLimits.at(type).validate(value.value);
		else if (limits.count(type)) limits.at(type).validate(value);
		else if (lists.count(type)) lists.at(type).validate(section, key, value); // 新增
		else if (sections.count(type)) {
			if (!targetIni->sections.count(value))
				throw std::string("\"" + type + "\"中声明的\"" + value + "\"未被实现");
			validateSection(value, targetIni->sections.at(value), type);
		}
	}
	catch (const std::string& e) {
		ERRORK(section, key) << e;
	}
	catch (const std::invalid_argument) {
		ERRORK(section, key) << value << "是非法参数";
	}
	catch (const std::out_of_range) {
		ERRORK(section, key) << value << "超过限制";
	}
	catch (...) {
		ERRORK(section, key) << "非预期的错误";
	}
}

int Checker::isInteger(const Value& value) {
	int base = 10;
	std::string buffer = value;
	if (*buffer.begin() == '$') {
		buffer = buffer.substr(1, buffer.size());
		base = 16;
	}
	else if (tolower(buffer.back()) == 'h') {
		buffer = buffer.substr(0, buffer.size() - 1);
		base = 16;
	}

	std::size_t pos;
	auto result = std::stoi(value, &pos);
	if (pos != buffer.size())
		throw std::string(value + "不是整数，非整数部分会被忽略");

	return result;
}

float Checker::isFloat(const Value& value) {
	std::string buffer = value;
	if (buffer.back() == '%')
		buffer = buffer.substr(0, buffer.size() - 1);
	
	if (*buffer.begin() == '.')
		buffer = "0" + buffer;

	std::size_t pos;
	auto result = std::stof(value, &pos);
	if (pos != buffer.size())
		throw std::string(value + "不是浮点数，非浮点数部分会被忽略");

	if (value.value.back() == '%')
		result /= 100;

	return result;
}

double Checker::isDouble(const Value& value) {
	std::string buffer = value;
	if (buffer.back() == '%')
		buffer = buffer.substr(0, buffer.size() - 1);

	std::size_t pos;
	auto result = std::stod(value, &pos);
	if (pos != buffer.size())
		throw std::string(value + "不是浮点数，非浮点数部分会被忽略");

	if (value.value.back() == '%')
		result /= 100;

	return result;
}

std::string Checker::isString(const Value& value) {
	if (value.value.size() > 512)
		throw std::string("值超过最大字数限制：" + value);

	return value;
}