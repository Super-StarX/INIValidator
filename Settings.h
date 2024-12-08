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
	std::string _KeyNotExist{ };				// 记录不存在的Key
	std::string _TypeNotExist{ };				// 记录不存在的Type
	std::string _DynamicKeyVariableError{ };	// 记录动态键变量错误
	std::string _DynamicKeyFormatError{ };		// 记录动态键格式错误

	// 整个文件方面
	std::string _UnusedGlobal{ };				// 检测未使用的全局节
	std::string _UnusedRegistry{ };				// 检测未使用的注册表
	std::string _SectionExsit{ };				// 检测注册表注册项是否实现

	// 节层面
	std::string _BracketClosed{ };				// 检测括号闭合
	std::string _DuplicateKey{ };				// 检测重复的Key
	std::string _SectionFormat{ };				// 检测节格式
	std::string _InheritanceFormat{ };			// 检测继承格式
	std::string _InheritanceSectionExsit{ };	// 检测继承内容是否实现

	// 键层面
	std::string _SpaceExistBeforeEqualSign{ };	// 检测等号前是否有空格
	std::string _SpaceExistAfterEqualSign{ };	// 检测等号后是否有空格

	// 值层面
	std::string _EmptyValue{ };					// 检测Value是否为空
};

#define KeyNotExist &Settings::_KeyNotExist
#define TypeNotExist &Settings::_TypeNotExist
#define DynamicKeyVariableError &Settings::_DynamicKeyVariableError
#define DynamicKeyFormatError &Settings::_DynamicKeyFormatError
#define UnusedGlobal &Settings::_UnusedGlobal
#define UnusedRegistry &Settings::_UnusedRegistry
#define SectionExsit &Settings::_SectionExsit
#define BracketClosed &Settings::_BracketClosed
#define BracketClosed &Settings::_BracketClosed
#define DuplicateKey &Settings::_DuplicateKey
#define SectionFormat &Settings::_SectionFormat
#define InheritanceFormat &Settings::_InheritanceFormat
#define InheritanceFormat &Settings::_InheritanceFormat
#define InheritanceSectionExsit &Settings::_InheritanceSectionExsit
#define SpaceExistBeforeEqualSign &Settings::_SpaceExistBeforeEqualSign
#define SpaceExistAfterEqualSign &Settings::_SpaceExistAfterEqualSign
#define EmptyValue &Settings::_EmptyValue