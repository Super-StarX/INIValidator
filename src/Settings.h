#pragma once
#include "IniFile.h"
#include <string>

class Settings {
public:
	static Settings* Instance;
	Settings(const IniFile& configFile);
	void load(const IniFile& configFile);

	// 配置文件方面
	std::string KeyNotExist{ };					// 记录不存在的Key
	std::string TypeNotExist{ };				// 记录不存在的Type
	std::string DynamicKeyVariableError{ };		// 记录动态键变量错误
	std::string DynamicKeyFormatError{ };		// 记录动态键格式错误

	// 整个文件方面
	std::string UnusedGlobal{ };				// 检测未使用的全局节
	std::string UnusedRegistry{ };				// 检测未使用的注册表
	std::string SectionExsit{ };				// 检测注册表注册项是否实现
	std::string UnreachableSection{ };			// 检测未收录没检测的节

	// 节层面
	std::string BracketClosed{ };				// 检测括号闭合
	std::string DuplicateKey{ };				// 检测重复的Key
	std::string SectionFormat{ };				// 检测节格式
	std::string InheritanceFormat{ };			// 检测继承格式
	std::string InheritanceBracketClosed{ };	// 检测继承括号闭合
	std::string InheritanceSectionExsit{ };		// 检测继承内容是否实现
	std::string InheritanceDuplicateKey{ };		// 检测继承键重复

	// 键层面
	std::string SpaceExistBetweenEqualSign{ };	// 检测等号前是否有空格
	std::string SpaceLostBetweenEqualSign{ };	// 检测等号后是否有空格

	// 值层面
	std::string EmptyValue{ };					// 检测Value是否为空
	std::string IllegalValue{ };				// 违法参数
	std::string OverlongValue{ };				// 数值大小超过限制
	std::string IntIllegal{ };					// 不是整数
	std::string FloatIllegal{ };				// 不是浮点数
	std::string OverlongString{ };				// 字符串超过最大字数限制

	std::string TypeCheckerTypeNotExist{ };		// 检查关联的类型不存在

	std::string NumberCheckerOverRange{ };

	std::string LimitCheckerPrefixIllegal{ };
	std::string LimitCheckerSuffixIllegal{ };
	std::string LimitCheckerValueIllegal{ };
	std::string LimitCheckerOverRange{ };

	std::string ListCheckerUnknownType{ };
	std::string ListCheckerRangeIllegal{ };
	std::string ListCheckerOverRange{ };
};

#define _KeyNotExist &Settings::KeyNotExist
#define _TypeNotExist &Settings::TypeNotExist
#define _DynamicKeyVariableError &Settings::DynamicKeyVariableError
#define _DynamicKeyFormatError &Settings::DynamicKeyFormatError

#define _UnusedGlobal &Settings::UnusedGlobal
#define _UnusedRegistry &Settings::UnusedRegistry
#define _SectionExsit &Settings::SectionExsit
#define _UnreachableSection &Settings::UnreachableSection

#define _BracketClosed &Settings::BracketClosed
#define _BracketClosed &Settings::BracketClosed
#define _DuplicateKey &Settings::DuplicateKey
#define _SectionFormat &Settings::SectionFormat
#define _InheritanceFormat &Settings::InheritanceFormat
#define _InheritanceBracketClosed &Settings::InheritanceBracketClosed
#define _InheritanceSectionExsit &Settings::InheritanceSectionExsit
#define _InheritanceDuplicateKey &Settings::InheritanceDuplicateKey

#define _SpaceExistBetweenEqualSign &Settings::SpaceExistBetweenEqualSign
#define _SpaceLostBetweenEqualSign &Settings::SpaceLostBetweenEqualSign

#define _EmptyValue &Settings::EmptyValue
#define _IllegalValue &Settings::IllegalValue
#define _OverlongValue &Settings::OverlongValue
#define _IntIllegal &Settings::IntIllegal
#define _FloatIllegal &Settings::FloatIllegal
#define _OverlongString &Settings::OverlongString

#define _TypeCheckerTypeNotExist &Settings::TypeCheckerTypeNotExist

#define _NumberCheckerOverRange &Settings::NumberCheckerOverRange

#define _LimitCheckerPrefixIllegal &Settings::LimitCheckerPrefixIllegal
#define _LimitCheckerSuffixIllegal &Settings::LimitCheckerSuffixIllegal
#define _LimitCheckerValueIllegal &Settings::LimitCheckerValueIllegal
#define _LimitCheckerOverRange &Settings::LimitCheckerOverRange

#define _ListCheckerUnknownType &Settings::ListCheckerUnknownType
#define _ListCheckerRangeIllegal &Settings::ListCheckerRangeIllegal
#define _ListCheckerOverRange &Settings::ListCheckerOverRange