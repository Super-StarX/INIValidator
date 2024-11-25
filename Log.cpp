#include "Log.h"
#include <iostream>
#include <iomanip>

Log* Log::Instance;

Log::Log(const std::string& logFileName) : running(true) {
    Instance = this;
    logFile.open(logFileName, std::ios::out | std::ios::trunc);
    if (!logFile.is_open())
        throw std::runtime_error("Unable to open log file: " + logFileName);

    writerThread = std::thread(&Log::processLogQueue, this);
}

Log::~Log() {
    stop();
    if (writerThread.joinable())
        writerThread.join();
    if (logFile.is_open())
        logFile.close();
}

LogStream Log::stream(Severity severity, int line) {
    return LogStream(this, severity, line);
}

LogStream Log::stream(Severity severity, const Section& section, const std::string& key) {
	const auto& value = section.at(key);
	return LogStream(this, severity, value.getFileName(), section.name, key, value.value, value.line);
}

LogStream Log::stream(Severity severity, const std::string& section, const std::string& filename, const int& line) {
	return LogStream(this, severity, filename, section, std::string(), std::string(), line);
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
    case Severity::DEFAULT: return "";
    case Severity::INFO:    return "\033[32m[建议]\033[0m";
    case Severity::WARNING: return "\033[33m[非法]\033[0m";
    case Severity::ERROR:   return "\033[31m[错误]\033[0m";
    default:__assume(0);
    }
}

std::string Log::getPlainSeverityLabel(Severity severity) {
    switch (severity) {
    case Severity::DEFAULT: return "";
    case Severity::INFO:    return "[建议]";
    case Severity::WARNING: return "[非法]";
    case Severity::ERROR:   return "[错误]";
    default:__assume(0);
    }
}

LogStream::LogStream(Log* logger, Severity severity, int line)
    : logger(logger), severity(severity), line(line) {}

LogStream::LogStream(Log* logger, Severity severity, std::string filename, std::string section, std::string key, std::string value, int line)
	: logger(logger), severity(severity), line(line), filename(filename), section(section), key(key), value(value) {}

LogStream::~LogStream() {
    if (severity == Severity::DEFAULT) {
        std::cerr << buffer.str() << std::endl;
        logger->writeLog(buffer.str());
    }
    else {
        std::ostringstream formattedMessage;
		std::ostringstream plainMessage;

		if (section.empty()) {
			// 控制台输出带颜色的内容
			formattedMessage << logger->getSeverityLabel(severity) << " 第" << line << "行\t| " << buffer.str();
			std::cerr << formattedMessage.str() << std::endl;

			// 文件写入无颜色内容
			plainMessage << logger->getPlainSeverityLabel(severity) << " 第" << line << "行\t| " << buffer.str();
			logger->writeLog(plainMessage.str());
		}
		else {
			std::string linenumber = std::format("第{}行", line);
			std::string pair = std::format("[{}] ", section);
			if (!key.empty())
				pair += std::format("{}={}", key, value);
			size_t blocksize = std::max(filename.size(), linenumber.size());

			formattedMessage << logger->getSeverityLabel(severity) << " " << std::left << std::setw(blocksize) << filename << std::setw(0) << " | " << pair
				<< std::endl << "[详情] " << std::left << std::setw(blocksize) << linenumber << std::setw(0) << " | " << buffer.str();
			std::cerr << formattedMessage.str() << std::endl;

			plainMessage << logger->getPlainSeverityLabel(severity) << " " << std::left << std::setw(blocksize) << filename << std::setw(0) << " | " << pair
				<< std::endl << "[详情] " << std::left << std::setw(blocksize) << linenumber << std::setw(0) << " | " << buffer.str();
			logger->writeLog(plainMessage.str());
		}
    }
}
