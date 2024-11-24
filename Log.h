#pragma once
#include "IniFile.h"
#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <memory>

#define LOG Log::Instance->stream(Severity::DEFAULT)
#define INFO(line) Log::Instance->stream(Severity::INFO, line)
#define WARNING(line) Log::Instance->stream(Severity::WARNING, line)
#define ERROR(line) Log::Instance->stream(Severity::ERROR, line)

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
    static Log* Instance; 
    Log(const std::string& logFileName);
    ~Log();

    LogStream stream(Severity severity, int line = -1);
    LogStream stream(Severity severity, Value value);

    void stop();

private:
    friend class LogStream;

    void processLogQueue();
    void writeLog(const std::string& message);
    std::string getSeverityLabel(Severity severity);
    std::string getPlainSeverityLabel(Severity severity);

    std::ofstream logFile;
    std::queue<std::string> logQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::thread writerThread;
    bool running;

};

// 流式日志操作
class LogStream {
public:
    LogStream(Log* logger, Severity severity, int line);
    ~LogStream(); // 析构时提交日志
    template <typename T>
    LogStream& operator<<(const T& value) {
        buffer << value;
        return *this;
    }
	LogStream& operator<<(const Value& value) {
		buffer << value.value;
		return *this;
	}
private:
    Log* logger;
    Severity severity;
    int line;
    std::ostringstream buffer;
};
