#include "CustomChecker.h"
#include <iostream>

// Script 构造函数
CustomChecker::Script::Script(PyObject* mod, PyObject* func)
	: module(mod), validateFunc(func) {
}

// Script 析构函数
CustomChecker::Script::~Script() {
	Py_XDECREF(validateFunc);
	Py_XDECREF(module);
}

// CustomChecker 构造函数
CustomChecker::CustomChecker(const std::string& scriptDir)
	: scriptDir_(scriptDir) {
	Py_Initialize();
	if (!Py_IsInitialized()) {
		throw std::runtime_error("Failed to initialize Python interpreter");
	}
	scanScriptDirectory(); // 初始化支持的脚本类型
}

// CustomChecker 析构函数
CustomChecker::~CustomChecker() {
	if (Py_IsInitialized()) {
		Py_Finalize();
	}
}

// 扫描脚本目录
void CustomChecker::scanScriptDirectory() {
	for (const auto& entry : std::filesystem::directory_iterator(scriptDir_)) {
		if (entry.is_regular_file()) {
			const auto& path = entry.path();
			if (path.extension() == ".py") {
				supportedTypes_.insert(path.stem().string());
			}
		}
	}
}

// 获取支持的脚本类型
const std::unordered_set<std::string>& CustomChecker::getSupportedTypes() const {
	return supportedTypes_;
}

// 获取或加载脚本
std::shared_ptr<CustomChecker::Script> CustomChecker::getOrLoadScript(const std::string& type) {
	if (scriptCache_.count(type)) {
		return scriptCache_[type];
	}

	if (!supportedTypes_.count(type)) {
		throw std::runtime_error("Script type not supported: " + type);
	}

	PyObject* pName = PyUnicode_DecodeFSDefault(type.c_str());
	PyObject* pModule = PyImport_Import(pName);
	Py_XDECREF(pName);

	if (!pModule) {
		throw std::runtime_error("Failed to load Python script: " + type);
	}

	PyObject* pFunc = PyObject_GetAttrString(pModule, "validate");
	if (!pFunc || !PyCallable_Check(pFunc)) {
		Py_XDECREF(pFunc);
		Py_XDECREF(pModule);
		throw std::runtime_error("Python validate function not found or not callable in script: " + type);
	}

	auto script = std::make_shared<Script>(pModule, pFunc);
	scriptCache_[type] = script;
	return script;
}

// 验证函数
std::string CustomChecker::validate(const std::string& section, const std::string& key, const std::string& value, const std::string& type) {
	try {
		auto script = getOrLoadScript(type);
		if (!script || !script->validateFunc) {
			throw std::runtime_error("Invalid or missing Python validate function for type: " + type);
		}

		// 准备参数
		PyObject* pArgs = PyTuple_Pack(4,
									   PyUnicode_FromString(section.c_str()),
									   PyUnicode_FromString(key.c_str()),
									   PyUnicode_FromString(value.c_str()),
									   PyUnicode_FromString(type.c_str()));

		// 调用 Python 函数
		PyObject* pResult = PyObject_CallObject(script->validateFunc, pArgs);
		Py_XDECREF(pArgs);

		if (!pResult) {
			throw std::runtime_error("Python function call failed");
		}

		// 获取返回值并转为字符串
		const char* resultStr = PyUnicode_AsUTF8(pResult);
		Py_XDECREF(pResult);

		if (!resultStr) {
			throw std::runtime_error("Python function returned non-string result");
		}

		return std::string(resultStr);
	}
	catch (const std::exception& e) {
		std::cerr << "Error in CustomChecker: " << e.what() << std::endl;
		return std::string();
	}
}
