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
	std::string origin{};
	bool isSectionName{ false };

	explicit LogData() = default;
	LogData(const int line) : line(line) {}
	LogData(const Section& section, const std::string& key) :section(section.name) {
		const auto& value = section.at(key);
		this->line = value.line;
		this->origin = value.origin;
		this->fileindex = value.fileIndex;
	}
	LogData(const std::string& origin, size_t fileindex, const int line, bool isSectionName = false)
		: fileindex(fileindex), origin(origin), line(line), isSectionName(isSectionName) {}

	bool operator<(const LogData& r) const {
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

	template<typename... Args>
	static void out(Args&&... args) {
		LogData l;
		stream<0>(Severity::DEFAULT, l, std::forward<Args>(args)...);
	}

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
	void summary(std::map<std::string, std::map<Severity, int>>& fileSeverityCount);

	template <auto Member, typename... Args>
	static void stream(Severity severity, const LogData& logdata, Args&&... args) {
		try {
			if constexpr (Member == 0) {
				static_assert(sizeof...(Args) > 0, "不允许不写参数的直接LOG输出");

				if constexpr (sizeof...(Args) > 1) {
					auto tuple = std::make_tuple(std::forward<Args>(args)...);
					auto content = std::apply(
						[&](const auto& first, const auto&... rest) {
						return std::vformat(first, std::make_format_args(rest...));
					},
						tuple);
					Log::Logs.emplace(severity, logdata, content);
				}
				else {
					auto content = std::vformat("{}", std::make_format_args(args...));
					Log::Logs.emplace(severity, logdata, content);
				}
			}
			else {
				if (Settings::Instance && !(Settings::Instance->*Member).empty()) {
					auto content = std::vformat(Settings::Instance->*Member, std::make_format_args(args...));
					Log::Logs.emplace(severity, logdata, content);
				}
			}
		}
		catch (const std::format_error& e) {
			std::cerr << "格式错误：" << e.what() << "\n输入参数：";

			if constexpr (Member != 0) 
				std::cerr << Settings::Instance->*Member << " ";

			((std::cerr << args << " "), ...);
			std::cerr << std::endl;
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