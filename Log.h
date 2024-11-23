#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

class Log {
public:
    enum class Severity {
        INFO,
        WARNING,
        ERROR
    };

    Log(const std::string& logFileName);
    ~Log();

    // 普通日志
    void log(const std::string& message);
    // 检查问题日志
    void logIssue(Severity severity, int line, const std::string& key, const std::string& value, const std::string& errorMessage);

    // 停止日志线程
    void stop();

private:
    std::ofstream logFile;
    std::queue<std::string> logQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::thread writerThread;
    std::atomic<bool> running;

    void processLogQueue();

    // 日志颜色编码
    std::string getSeverityColor(Severity severity);
};