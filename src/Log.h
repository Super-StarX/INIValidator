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
	char fileindex{ 0 };
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
	LogData(const std::string& section, const char& fileindex, const int& line) : fileindex(fileindex), section(section), line(line){}
	
	bool operator<(const LogData& r){
		return fileindex == r.fileindex ? line < r.line : fileindex < r.fileindex;
	}
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
		stream<Member>(Severity::DEFAULT, logdata, std::forward<Args>(args)...);
	}

	template <auto Member, typename... Args>
	static void info(const LogData& logdata, Args&&... args) {
		stream<Member>(Severity::INFO, logdata, std::forward<Args>(args)...);
	}

	template <auto Member, typename... Args>
	static void warning(const LogData& logdata, Args&&... args) {
		stream<Member>(Severity::WARNING, logdata, std::forward<Args>(args)...);
	}

	template <auto Member, typename... Args>
	static void error(const LogData& logdata, Args&&... args) {
		stream<Member>(Severity::ERROR, logdata, std::forward<Args>(args)...);
	}

private:
	std::ofstream logFile;
	std::mutex fileMutex;
	static std::string getSeverityLabel(Severity severity);
	static std::string getPlainSeverityLabel(Severity severity);
	void writeLog(const std::string& log);

	template <auto Member, typename... Args>
	static void stream(Severity severity, const LogData& logdata, Args&&... args) {
		try {
			auto format = std::vformat(Settings::Instance->*Member, std::make_format_args(args...));
			Log::Logs.emplace(severity, logdata, format);
		}
		catch (const std::format_error& e) {
			std::cerr << "格式错误：" << e.what() << std::endl;
		}
	}
};

// 日志条
class LogStream {
public:
	explicit LogStream() = default;
	LogStream(Severity severity, const LogData& logdata, std::string buffer);
	
	std::string getFileMessage() const;
	std::string getPrintMessage() const;

private:
	bool operator<(const LogStream& r) {
		return data < r.data;
	}

	Severity severity;
	LogData data;
	std::string buffer;
};