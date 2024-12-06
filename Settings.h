#pragma once
#include "IniFile.h"
#include <string>
#include <unordered_map>
#include <iostream>
#include <stdexcept>
#include <vector>
#define TO_STRING(x) #x
class Settings {
public:
	// 构造函数，读取 ini 文件
	Settings(const IniFile& configFile);
	void load(const IniFile& configFile);
private:
	// 配置文件方面
	bool RecordKeyNotExist{ false };				// 记录不存在的Key
	bool RecordTypeNotExist{ true };				// 记录不存在的Type
	bool RecordDynamicKeyVariableError{ true };		// 记录动态键变量错误
	bool RecordDynamicKeyFormatError{ true };		// 记录动态键格式错误

	// 整个文件方面
	bool CheckUnusedGlobal{ true };					// 检测未使用的全局节
	bool CheckUnusedRegistry{ true };				// 检测未使用的注册表
	bool CheckSectionExsit{ true };					// 检测注册表注册项是否实现

	// 节层面
	bool CheckBracketClosed{ true };				// 检测括号闭合
	bool CheckDuplicateKey{ true };					// 检测重复的Key
	bool CheckSectionFormat{ true };				// 检测节格式
	bool CheckInheritanceFormat{ true };			// 检测继承格式
	bool CheckInheritanceSectionExsit{ true };		// 检测继承内容是否实现

	// 键层面
	bool CheckSpaceExistBeforeEqualSign{ false };	// 检测等号前是否有空格
	bool CheckSpaceExistAfterEqualSign{ false };	// 检测等号后是否有空格

	// 值层面
	bool CheckEmptyValue{ true };					// 检测Value是否为空
};