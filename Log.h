#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <memory>
#define INFO(line) Log::Instance->createStream(Severity::INFO, line)
#define WARNING(line) Log::Instance->createStream(Severity::WARNING, line)
#define ERROR(line) Log::Instance->createStream(Severity::ERROR, line)

// 日志级别
enum class Severity {
    INFO,
    WARNING,
    ERROR
};

class LogStream; // 前向声明

// 日志类
class Log {
public:
    static Log* Instance; // 静态实例
    Log(const std::string& logFileName);
    ~Log();

    LogStream createStream(Severity severity, int line);

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

// 中间类：用于流式日志操作
class LogStream {
public:
    LogStream(Log* logger, Severity severity, int line);
    ~LogStream(); // 析构时提交日志
    template <typename T>
    LogStream& operator<<(const T& value) {
        buffer << value;
        return *this;
    }

private:
    Log* logger;
    Severity severity;
    int line;
    std::ostringstream buffer;
};
