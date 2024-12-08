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
	std::string KeyNotExist{ };				// 记录不存在的Key
	std::string TypeNotExist{ };				// 记录不存在的Type
	std::string DynamicKeyVariableError{ };	// 记录动态键变量错误
	std::string DynamicKeyFormatError{ };		// 记录动态键格式错误

	// 整个文件方面
	std::string UnusedGlobal{ };				// 检测未使用的全局节
	std::string UnusedRegistry{ };				// 检测未使用的注册表
	std::string SectionExsit{ };				// 检测注册表注册项是否实现

	// 节层面
	std::string BracketClosed{ };				// 检测括号闭合
	std::string DuplicateKey{ };				// 检测重复的Key
	std::string SectionFormat{ };				// 检测节格式
	std::string InheritanceFormat{ };			// 检测继承格式
	std::string InheritanceSectionExsit{ };	// 检测继承内容是否实现

	// 键层面
	std::string SpaceExistBeforeEqualSign{ };	// 检测等号前是否有空格
	std::string SpaceExistAfterEqualSign{ };	// 检测等号后是否有空格

	// 值层面
	std::string EmptyValue{ };					// 检测Value是否为空
};