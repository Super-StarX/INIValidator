#include "Log.h"

Log::Log(const std::string& logFileName) : running(true) {
    logFile.open(logFileName, std::ios::out | std::ios::app);
    if (!logFile.is_open())
        throw std::runtime_error("Unable to open log file: " + logFileName);

    // 启动日志写入线程
    writerThread = std::thread(&Log::processLogQueue, this);
}

Log::~Log() {
    stop();
    if (writerThread.joinable())
        writerThread.join();
    if (logFile.is_open())
        logFile.close();
}

void Log::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(queueMutex);
    logQueue.push(message);
    condition.notify_one();
}

void Log::logIssue(Severity severity, int line, const std::string& key, const std::string& value, const std::string& errorMessage) {
    std::ostringstream formattedMessage;
    std::string color = getSeverityColor(severity);

    // 格式化问题日志
    formattedMessage << color;
    switch (severity) {
    case Severity::INFO:    formattedMessage << "[建议]"; break;
    case Severity::WARNING: formattedMessage << "[非法]"; break;
    case Severity::ERROR:   formattedMessage << "[错误]"; break;
    }
    formattedMessage << "\033[0m"; // 重置颜色
    formattedMessage << " Line " << line << " | " << key << " = " << value << " : " << errorMessage;

    // 打印到控制台
    std::cerr << formattedMessage.str() << std::endl;

    // 添加到日志文件队列
    std::lock_guard<std::mutex> lock(queueMutex);
    logQueue.push(formattedMessage.str());
    condition.notify_one();
}

void Log::stop() {
    running = false;
    condition.notify_one(); // 唤醒写入线程以便结束
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

std::string Log::getSeverityColor(Severity severity) {
    switch (severity) {
    case Severity::INFO:    return "\033[32m"; // 绿色
    case Severity::WARNING: return "\033[33m"; // 黄色
    case Severity::ERROR:   return "\033[31m"; // 红色
    }
    return "\033[0m";
}
