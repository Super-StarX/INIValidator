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

	// 判断是否是包含数学表达式的字符串
	inline bool isNumber(const std::string& s) {
		return !s.empty() && s.find_first_not_of("0123456789.-") == std::string::npos;
	}

	// 判断是否是包含数学表达式的字符串
	inline bool isExpression(const std::string& str) {
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