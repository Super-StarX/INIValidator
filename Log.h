#pragma once
#include "IniFile.h"
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

// 日志类
class LogStream;
class Log {
public:
	friend LogStream;
    static Log* Instance; 
	static std::set<LogStream, LogStream> Logs;

    Log();

	void output(const std::string& logFileName);

	template<Severity severity = Severity::DEFAULT>
	void operator()(int line) {
		Log::Logs.emplace_back(this, severity, line);
	};

	template<Severity severity = Severity::DEFAULT>
	void operator()(const Section& section, const std::string& key) {
		const auto& value = section.at(key);
		Log::Logs.emplace_back(this, severity, value.fileIndex, section.name, key, value.value, value.line);
	};

	template<Severity severity = Severity::DEFAULT>
	void operator()(const std::string& section, const size_t& fileindex, const int& line) {
		Log::Logs.emplace_back(this, severity, fileindex, section, std::string(), std::string(), line);
	};

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
	LogStream(Log* logger, Severity severity, int line, ...);
	LogStream(Log* logger, Severity severity, size_t fileindex, 
		std::string section, std::string key, std::string value, int line, ...);
	~LogStream();
	
	std::string getFileMessage() const;
	std::string getPrintMessage() const;
	int getline() const { return line; }
	size_t getindex() const { return fileindex; }

private:
	bool operator()(const LogStream& l, const LogStream& r);
	std::string formatString(const char* format, va_list args);

	Log* logger;
	Severity severity;
	int line;
	size_t fileindex{ 0 };
	std::string section{};
	std::string key{};
	std::string value{};
	std::string buffer;
};