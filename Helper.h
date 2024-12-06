#pragma once
#include <vector>
#include <sstream>
#include <string>
#include <stack>

namespace string {
	inline std::vector<std::string> split(const std::string& str, char delimiter = ',') {
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(str);
		while (std::getline(tokenStream, token, delimiter))
			tokens.push_back(token);
		return tokens;
	}

	inline std::vector<std::string> splitAsString(const std::string& input, const std::string& delimiter = "||") {
		std::vector<std::string> result;
		size_t start = 0, end = 0;

		// 使用 delimiter 查找并分割字符串
		while ((end = input.find(delimiter, start)) != std::string::npos) {
			result.push_back(input.substr(start, end - start));
			start = end + delimiter.length();
		}

		// 添加最后一部分
		if (start < input.length())
			result.push_back(input.substr(start));

		return result;
	}

	// 判断是否是包含数学表达式的字符串
	inline bool isNumber(const std::string& s) {
		return !s.empty() && s.find_first_not_of("0123456789.-") == std::string::npos;
	}

	// 判断是否是包含数学表达式的字符串
	inline bool isExpression(const std::string& str) {
		return str.find_first_of("+-*/()") != std::string::npos;
	}

	inline bool isBool(const std::string& str) {
		char c = str.front();
		return str.find_first_of("+-*/()") != std::string::npos;
	}
}

namespace math {
	inline int precedence(char op) {
		if (op == '+' || op == '-') return 1;
		if (op == '*' || op == '/') return 2;
		return 0;
	}

	inline double applyOperation(double a, double b, char op) {
		switch (op) {
		case '+': return a + b;
		case '-': return a - b;
		case '*': return a * b;
		case '/':
			if (b != 0) return a / b; 
			throw std::string("除零错误:" + std::to_string(a) + "/" + std::to_string(b));
		default:
			throw std::string("异常操作符:" + op);
		}
	}
}