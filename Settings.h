#pragma once
#include "IniFile.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
class Settings {
public:
	static Settings* Instance;
	Settings(const IniFile& configFile);
	void load(const IniFile& configFile);

	// 配置文件方面
	std::string recordKeyNotExist{ };				// 记录不存在的Key
	std::string recordTypeNotExist{ };				// 记录不存在的Type
	std::string recordDynamicKeyVariableError{ };	// 记录动态键变量错误
	std::string recordDynamicKeyFormatError{ };		// 记录动态键格式错误

	// 整个文件方面
	std::string checkUnusedGlobal{ };				// 检测未使用的全局节
	std::string checkUnusedRegistry{ };				// 检测未使用的注册表
	std::string checkSectionExsit{ };				// 检测注册表注册项是否实现

	// 节层面
	std::string checkBracketClosed{ };				// 检测括号闭合
	std::string checkDuplicateKey{ };				// 检测重复的Key
	std::string checkSectionFormat{ };				// 检测节格式
	std::string checkInheritanceFormat{ };			// 检测继承格式
	std::string checkInheritanceSectionExsit{ };	// 检测继承内容是否实现

	// 键层面
	std::string checkSpaceExistBeforeEqualSign{ };	// 检测等号前是否有空格
	std::string checkSpaceExistAfterEqualSign{ };	// 检测等号后是否有空格

	// 值层面
	std::string checkEmptyValue{ };					// 检测Value是否为空
};