#pragma once
#include <Python.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <filesystem>

class CustomChecker {
public:
	// 构造函数，指定脚本目录
	explicit CustomChecker(const std::string& scriptDir);

	// 析构函数，释放 Python 资源
	~CustomChecker();

	// 检查指定脚本是否存在，并调用其验证逻辑
	std::string validate(const std::string& section, const std::string& key, const std::string& value, const std::string& type);

	// 返回支持的脚本类型
	const std::unordered_set<std::string>& getSupportedTypes() const;

private:
	// 脚本的结构体
	struct Script {
		PyObject* module;       // Python 模块
		PyObject* validateFunc; // validate 函数指针

		Script(PyObject* mod, PyObject* func);
		~Script();
	};

	std::string scriptDir_;                                     // 脚本目录
	std::unordered_map<std::string, std::shared_ptr<Script>> scriptCache_; // 缓存已加载的脚本
	std::unordered_set<std::string> supportedTypes_;            // 支持的脚本类型集合

	// 加载或从缓存中获取指定的脚本
	std::shared_ptr<Script> getOrLoadScript(const std::string& type);

	// 扫描脚本目录，初始化支持的脚本类型
	void scanScriptDirectory();
};