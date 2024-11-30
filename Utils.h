#pragma once
#include <vector>
#include <sstream>
#include <string>
namespace string {
	std::vector<std::string> split(const std::string& str, char delimiter) {
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(str);
		while (std::getline(tokenStream, token, delimiter)) {
			tokens.push_back(token);
		}
		return tokens;
	}
}

namespace number {
	// 判断是否是包含数学表达式的字符串
	bool isExpression(const std::string& str) {
		return str.find_first_of("+-*/()") != std::string::npos;
	}
}
