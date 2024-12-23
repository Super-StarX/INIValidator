#include "CustomChecker.h"
#include "Log.h"
#include <iostream>

std::unordered_map<std::string, Section> CustomChecker::globalSections_;
PyObject* CustomChecker::pyGlobalSections_ = nullptr;
std::mutex globalSectionsMutex_;

PyObject* CustomChecker::py_get_section(PyObject* self, PyObject* args) {
	const char* name;

	// 从参数中解析 Section 名称
	if (!PyArg_ParseTuple(args, "s", &name)) {
		PyErr_SetString(PyExc_ValueError, "Expected a string argument for section name");
		return nullptr;
	}

	try {
		// 获取 Section
		auto it = globalSections_.find(std::string(name));
		if (it == globalSections_.end()) {
			Log::out("找不到section[{}]: ", name);
			return PyDict_New();
		}

		PyObject* pyDict = PyDict_New();
		for (const auto& [key, value] : it->second.section) {
			PyObject* pyValue = PyUnicode_FromString(value.value.c_str());
			PyDict_SetItemString(pyDict, key.c_str(), pyValue);
			Py_DECREF(pyValue);
		}
		return pyDict;
	}
	catch (const std::exception& e) {
		Log::out("获取section[{}]失败，错误信息: ", name, e.what());
		PyErr_SetString(PyExc_KeyError, e.what());
		return nullptr;
	}
}

static PyMethodDef CustomCheckerMethods[] = {
	{"get_section", CustomChecker::py_get_section, METH_VARARGS, "Get a Section by name as a dictionary"},
	{nullptr, nullptr, 0, nullptr}  // 哨兵，用于结束方法列表
};

static struct PyModuleDef customCheckerModule = {
	PyModuleDef_HEAD_INIT,
	"iv",  // 模块名
	nullptr,           // 模块文档
	-1,                // 模块状态
	CustomCheckerMethods  // 模块方法列表
};

// 模块初始化函数
PyMODINIT_FUNC PyInit_iv(void) {
	return PyModule_Create(&customCheckerModule);
}

CustomChecker::Script::Script(PyObject* mod, PyObject* func)
	: module(mod), func(func) {
}

CustomChecker::Script::~Script() {
	Py_XDECREF(func);
	Py_XDECREF(module);
}

CustomChecker::CustomChecker(const std::string& scriptDir, const IniFile& targetIni)
	: scriptDir_(scriptDir) {
	globalSections_ = targetIni.sections;
	// 注册模块到 Python 解释器
	if (PyImport_AppendInittab("iv", PyInit_iv) == -1) {
		Log::out("Failed to add iv module");
		return;
	}
	Py_Initialize();
	if (!Py_IsInitialized()) {
		Log::out("初始化Python解释器失败，脚本名: {}", scriptDir);
		return;
	}
	PyRun_SimpleString("import locale; locale.setlocale(locale.LC_ALL, 'zh_CN.UTF-8')");
	scanScriptDirectory(); // 初始化支持的脚本类型
	PyRun_SimpleString("import iv");
}

CustomChecker::~CustomChecker() {
	scriptCache_.clear();
	if (Py_IsInitialized())
		Py_Finalize();
}

void CustomChecker::reportResult(PyObject* pMessage, PyObject* pCode, const Section& section, const std::string& key) {
	std::string message = PyUnicode_AsUTF8(pMessage);
	int code = static_cast<int>(PyLong_AsLong(pCode));
	if (code != -1) {
		switch (static_cast<Severity>(code)) {
		case Severity::DEFAULT:
			Log::print<0>({ section ,key }, message);
			break;
		case Severity::INFO:
			Log::info<0>({ section ,key }, message);
			break;
		case Severity::WARNING:
			Log::warning<0>({ section ,key }, message);
			break;
		case Severity::ERROR:
			Log::error<0>({ section ,key }, message);
			break;
		default:
			Log::out("非预期的状态码:{}", code);
			Log::out(message);
			break;
		}
	}
}

// 获取或加载脚本
std::shared_ptr<CustomChecker::Script> CustomChecker::getOrLoadScript(const std::string& type) {
	if (scriptCache_.contains(type))
		return scriptCache_[type];

	if (!supportedTypes_.contains(type)) {
		Log::out("脚本类型不支持: {}", type);
		return nullptr;
	}

	// 获取当前工作目录并拼接 Scripts 子目录
	std::filesystem::path script_dir = std::filesystem::current_path() / "Scripts";

	// 添加 Scripts 子目录到 Python 模块搜索路径
	PyObject* sys_path = PySys_GetObject("path");  // borrowed reference
	PyObject* path = PyUnicode_FromString(script_dir.string().c_str());
	PyList_Append(sys_path, path);
	Py_DECREF(path);

	PyObject* pModule = PyImport_ImportModule(type.c_str());
	if (!pModule) {
		Log::out("加载脚本失败: {}", type);
		return nullptr;
	}

	PyObject* pFunc = PyObject_GetAttrString(pModule, "validate");
	if (!pFunc || !PyCallable_Check(pFunc)) {
		Py_XDECREF(pFunc);
		Py_XDECREF(pModule);
		Log::out("无法在脚本中找到\"validate\"函数或者函数无法被调用: {}", type);
		return nullptr;
	}

	auto script = std::make_shared<Script>(pModule, pFunc);
	scriptCache_[type] = script;
	return script;
}

// 扫描脚本目录
void CustomChecker::scanScriptDirectory() {
	if (!std::filesystem::exists(scriptDir_) || !std::filesystem::is_directory(scriptDir_))
		return;

	for (const auto& entry : std::filesystem::directory_iterator(scriptDir_)) {
		if (entry.is_regular_file()) {
			const auto& path = entry.path();
			if (path.extension() == ".py")
				supportedTypes_.insert(path.stem().string());
		}
	}
}

// 验证函数
void CustomChecker::validate(const Section& section, const std::string& key, const Value& value, const std::string& type) {
	try {
		auto script = getOrLoadScript(type);
		if (!script || !script->func)
			return;

		// 转换 Section 为 Python 字典
		PyObject* pySection = PyDict_New();
		for (const auto& [k, v] : section.section) {
			PyObject* pyKey = PyUnicode_FromString(k.c_str());
			PyObject* pyValue = PyUnicode_FromString(v.value.c_str());
			PyDict_SetItem(pySection, pyKey, pyValue);
			Py_XDECREF(pyKey);
			Py_XDECREF(pyValue);
		}

		// 准备参数
		PyObject* pArgs = PyTuple_Pack(4,
									   pySection,
									   PyUnicode_FromString(key.c_str()),
									   PyUnicode_FromString(value.value.c_str()),
									   PyUnicode_FromString(type.c_str()));

		// 调用 Python 函数
		PyObject* pResult = PyObject_CallObject(script->func, pArgs);
		Py_XDECREF(pArgs);

		if (!pResult)
			return Log::out("Python函数调用失败: {}", type);

		// 解析结果
		if (!PyTuple_Check(pResult) || PyTuple_Size(pResult) != 2) {
			Py_XDECREF(pResult);
			return Log::out("Python函数必须返回一个(int, string)的元组类型: {}", type);
		}

		PyObject* pCode = PyTuple_GetItem(pResult, 0);
		PyObject* pMessage = PyTuple_GetItem(pResult, 1);

		if (!PyUnicode_Check(pMessage) || !PyLong_Check(pCode)) {
			Py_XDECREF(pResult);
			return Log::out("Python函数必须返回一个(int, string)的元组类型: {}", type);
		}

		reportResult(pMessage, pCode, section, key);

		Py_XDECREF(pResult);
	}
	catch (const std::exception& e) {
		Log::out("自定义检查器出现错误: {}", e.what());
	}
}
