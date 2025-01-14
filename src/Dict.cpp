#include "Dict.h"
#include "ProgressBar.h"
#include "Checker.h"
#include "IniFile.h"
#include "Log.h"
#include "Helper.h"

Dict::Dict(const Section& config) {
	for (const auto& [key, value] : config) {
		if (key.find('(') != std::string::npos && key.find(')') != std::string::npos)
			dynamicKeys.push_back(key);

		section[key] = parseTypeValue(value);
		keys.insert(key);
	}
}

void Dict::validateSection(const Section& object, const std::string& type) {
	if (object.isScanned)
		return;

	Progress::update();
	const_cast<Section&>(object).isScanned = true;

	for (const auto& dynamicKey : this->dynamicKeys) {
		try {
			auto keys = generateKey(dynamicKey, object);
			for (const auto& key : keys) {
				if (object.contains(key)) {
					this->keys.insert(key);
					this->validate(dynamicKey, key, object, object.at(key));
				}
			}
		}
		catch (const std::string& e) {
			Log::warning<_DynamicKeyVariableError>(object.section.begin()->second.line, e);
		}
		catch (const std::invalid_argument) {
			// 不做任何处理
		}
	}

	for (const auto& [key, value] : object) {
		if (!this->contains(key)) {
			if (!keys.contains(key))
				Log::info<_KeyNotExist>({ object, key }, key);
			continue;
		}

		this->validate(key, key, object, value);
	}
}

void Dict::validate(const Section::Key& key, const Section::Key& vkey, const Section& object, const Value& value) {
	if (value.filetype.empty() || this->at(key).file != value.filetype)
		return;

	for (const auto& type : this->at(key).types)
		Checker::Instance->validate(object, vkey, value, type);
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

// 用于生成动态key
std::vector<std::string> Dict::generateKey(const std::string& dynamicKey, const Section& object) {
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
double Dict::evaluateExpression(const std::string& expr, const Section& object) {
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

double Dict::parseValue(size_t& i, const std::string& expr, const Section& object) {
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
	if (!string::isNumber(value)) {
		Log::error<_DynamicKeyVariableError>({ object,value }, value);
		return 0;
	}

	return std::stod(value);
}

void Dict::applyOperation(std::stack<double>& values, std::stack<char>& operators) {
	double b = values.top(); values.pop();
	double a = values.top(); values.pop();
	char op = operators.top(); operators.pop();
	values.push(math::applyOperation(a, b, op));
}
