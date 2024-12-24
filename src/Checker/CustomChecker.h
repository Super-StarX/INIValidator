#pragma once
#include "IniFile.h"
#include <Python.h>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <filesystem>

class CustomChecker {
public:
	explicit CustomChecker(const std::string& scriptDir, const IniFile& targetIni);
	~CustomChecker();

	void reportResult(PyObject* pMessage, PyObject* pCode, const Section& section, const std::string& key);
	void validate(const Section& section, const std::string& key, const Value& value, const std::string& type);
	bool contains(const std::string& type) const {
		return supportedTypes_.contains(type);
	}

	static PyObject* py_get_section(PyObject* self, PyObject* args);
	static PyObject* py_get_section_value(PyObject* self, PyObject* args);

private:
	// 脚本的结构体
	struct Script {
		PyObject* module; // Python 模块
		PyObject* func; // validate 函数指针

		Script(PyObject* mod, PyObject* func);
		~Script();
	};

	static std::unordered_map<std::string, Section> globalSections_;		// 全局Section存储

	std::string scriptDir_;													// 脚本目录
	std::unordered_map<std::string, std::shared_ptr<Script>> scriptCache_;	// 缓存已加载的脚本
	std::unordered_set<std::string> supportedTypes_;						// 支持的脚本类型集合

	std::shared_ptr<Script> getOrLoadScript(const std::string& type); 
	void scanScriptDirectory(const std::string& path);
};