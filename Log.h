#pragma once
#include "IniFile.h"
#include "Settings.h"
#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>

// 日志级别
enum class Severity {
    DEFAULT,
    INFO,
    WARNING,
    ERROR
};

struct LogData {
	int line{ -2 };
	size_t fileindex{ 0 };
	std::string section{};
	std::string key{};
	std::string value{};

	explicit LogData() = default;
	LogData(const int line) : line(line) {}
	LogData(const Section& section, const std::string& key) : key(key), section(section.name) {
		const auto& value = section.at(key);
		line = value.line;
		this->value = value.line;
		fileindex = value.fileIndex;
	}
	LogData(const std::string& section, const size_t& fileindex, const int& line) : fileindex(fileindex), section(section), line(line){}

};

// 日志类
class LogStream;
class Log {
public:
	friend LogStream;
    static Log* Instance; 
	static std::set<LogStream> Logs;

    Log();

	void output(const std::string& logFileName);

	template <auto Member, typename... Args>
	static void print(const LogData& logdata, Args&&... args) {
		try {
			Log::Logs.emplace_back(Severity::DEFAULT, logdata, std::vformat(Settings::Instance->*Member, std::make_format_args(args...));
		}
		catch (const std::format_error& e) {
			std::cerr << "格式错误：" << e.what() << std::endl;
		}
	}

	template <auto Member, typename... Args>
	static void info(const LogData& logdata, Args&&... args) {
		try {
			Log::Logs.emplace_back(Severity::INFO, logdata, std::vformat(Settings::Instance->*Member, std::make_format_args(args...));
		}
		catch (const std::format_error& e) {
			std::cerr << "格式错误：" << e.what() << std::endl;
		}
	}

	template <auto Member, typename... Args>
	static void warning(const LogData& logdata, Args&&... args) {
		try {
			Log::Logs.emplace_back(Severity::WARNING, logdata, std::vformat(Settings::Instance->*Member, std::make_format_args(args...));
		}
		catch (const std::format_error& e) {
			std::cerr << "格式错误：" << e.what() << std::endl;
		}
	}

	template <auto Member, typename... Args>
	static void error(const LogData& logdata, Args&&... args) {
		try {
			Log::Logs.emplace_back(Severity::ERROR, logdata, std::vformat(Settings::Instance->*Member, std::make_format_args(args...));
		}
		catch (const std::format_error& e) {
			std::cerr << "格式错误：" << e.what() << std::endl;
		}
	}



private:
	std::ofstream logFile;
	std::mutex logMutex; // 保护Logs
	std::mutex fileMutex; // 保护logFile
	static std::string getSeverityLabel(Severity severity);
	static std::string getPlainSeverityLabel(Severity severity);
	void writeLog(const std::string& log); // 写入文件
};

// 日志条
class LogStream {
public:
	explicit LogStream() = default;
	LogStream(Severity severity, const LogData& logdata, std::string buffer);
	
	std::string getFileMessage() const;
	std::string getPrintMessage() const;

private:
	bool operator<(const LogStream& r);

	Severity severity;
	LogData data;
	std::string buffer;
};