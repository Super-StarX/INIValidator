#pragma once
#include "IniFile.h"
#include "Settings.h"
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <set>

// 日志级别
enum class Severity : int {
	DEFAULT,
	INFO,
	WARNING,
	ERROR,
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
		this->value = value;
		fileindex = value.fileIndex;
	}
	LogData(const std::string& section, size_t fileindex, const int line) : fileindex(fileindex), section(section), line(line){}
	
	bool operator<(const LogData& r) const{
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
		stream<Member>(Severity::DEFAULT, logdata, std::make_format_args(args...));
	}

	template <auto Member, typename... Args>
	static void info(const LogData& logdata, Args&&... args) {
		stream<Member>(Severity::INFO, logdata, std::make_format_args(args...));	
	}

	template <auto Member, typename... Args>
	static void warning(const LogData& logdata, Args&&... args) {
		stream<Member>(Severity::WARNING, logdata, std::make_format_args(args...));
	}

	template <auto Member, typename... Args>
	static void error(const LogData& logdata, Args&&... args) {
		stream<Member>(Severity::ERROR, logdata, std::make_format_args(args...));
	}

private:
	std::ofstream logFile;
	std::mutex fileMutex;
	static std::string getSeverityLabel(Severity severity);
	static std::string getPlainSeverityLabel(Severity severity);
	void writeLog(const std::string& log);
	void summary(std::map<std::string, std::map<Severity, int>>& fileSeverityCount);

	template <auto Member, typename... Args>
	static void stream(Severity severity, const LogData& logdata, auto args) {
		try {
			if (!(Settings::Instance->*Member).empty()) {
				auto format = std::vformat(Settings::Instance->*Member, args);
				Log::Logs.emplace(severity, logdata, format);
			}
		}
		catch (const std::format_error& e) {
			std::cerr << "格式错误：" << e.what() << std::endl;
		}
	}
};

// 日志条
class LogStream {
public:
	friend Log;
	explicit LogStream() = default;
	LogStream(Severity severity, const LogData& logdata, std::string buffer);
	
	std::string getFileMessage() const;
	std::string getPrintMessage() const;

	bool operator<(const LogStream& r) const {
		return data < r.data;
	}

private:
	std::string generateLogMessage() const;

	Severity severity;
	LogData data;
	std::string buffer;
};