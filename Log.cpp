#include "Log.h"
#include <iostream>

// 静态实例初始化
Log* Log::Instance;

Log::Log(const std::string& logFileName) : running(true) {
    Instance = this;
    logFile.open("app.log", std::ios::out | std::ios::trunc);
    if (!logFile.is_open())
        throw std::runtime_error("Unable to open log file: app.log");

    writerThread = std::thread(&Log::processLogQueue, this);
}

Log::~Log() {
    stop();
    if (writerThread.joinable())
        writerThread.join();
    if (logFile.is_open())
        logFile.close();
}

LogStream Log::createStream(Severity severity, int line) {
    return LogStream(this, severity, line);
}

void Log::stop() {
    running = false;
    condition.notify_one();
}

void Log::processLogQueue() {
    while (running || !logQueue.empty()) {
        std::unique_lock<std::mutex> lock(queueMutex);
        condition.wait(lock, [this]() { return !logQueue.empty() || !running; });

        while (!logQueue.empty()) {
            logFile << logQueue.front() << std::endl;
            logQueue.pop();
        }
    }
}

void Log::writeLog(const std::string& message) {
    std::lock_guard<std::mutex> lock(queueMutex);
    logQueue.push(message);
    condition.notify_one();
}

std::string Log::getSeverityLabel(Severity severity) {
    switch (severity) {
    case Severity::INFO:    return "\033[32m[建议]\033[0m";
    case Severity::WARNING: return "\033[33m[非法]\033[0m";
    case Severity::ERROR:   return "\033[31m[错误]\033[0m";
    }
    return "[未知]";
}

std::string Log::getPlainSeverityLabel(Severity severity) {
    switch (severity) {
    case Severity::INFO:    return "[建议]";
    case Severity::WARNING: return "[非法]";
    case Severity::ERROR:   return "[错误]";
    }
    return "[未知]";
}

// LogStream 实现

LogStream::LogStream(Log* logger, Severity severity, int line)
    : logger(logger), severity(severity), line(line) {}

LogStream::~LogStream() {
    std::ostringstream formattedMessage;

    // 控制台输出带颜色的内容
    formattedMessage << logger->getSeverityLabel(severity) << " Line " << line << " | " << buffer.str();
    std::cerr << formattedMessage.str() << std::endl;

    // 文件写入无颜色内容
    std::ostringstream plainMessage;
    plainMessage << logger->getPlainSeverityLabel(severity) << " Line " << line << " | " << buffer.str();
    logger->writeLog(plainMessage.str());
}
