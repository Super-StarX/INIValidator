#pragma once
#include <string>
#include <vector>
#include <sstream>

namespace string {
	inline static std::vector<std::string> split(const std::string& str, char delimiter = ',') {
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(str);
		while (std::getline(tokenStream, token, delimiter))
			tokens.push_back(token);
		return tokens;
	}

	inline static std::vector<std::string> splitAsString(const std::string& input, const std::string& delimiter = "||") {
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

	inline static std::string clamp(const std::string& str, const size_t length) {
		if (str.size() > length)
			return str.substr(0, length - 3) + "..."; // 超出部分用省略号

		return str + std::string(length - str.size(), ' '); // 补齐空格
	}

	// 去除注释
	inline std::string removeInlineComment(const std::string& str) {
		size_t commentPos = str.find(';');
		return commentPos != std::string::npos ? str.substr(0, commentPos) : str;
	}

	// 去除字符串开头结尾
	inline std::string trim(const std::string& str) {
		size_t start = str.find_first_not_of(" \t\r\n");
		size_t end = str.find_last_not_of(" \t\r\n");
		return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
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
		return c == '1' || c == 'y' || c == 'Y' || c == 't' || c == 'T';
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