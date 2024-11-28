#include "Log.h"
#include <iostream>
#include <iomanip>

Log* Log::Instance;
std::vector<LogStream> Log::Logs;
bool Log::CanOutput = false;

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

LogStream Log::logstream(Severity severity) {
	return LogStream(this, severity, -2);
}

LogStream& Log::stream(Severity severity, int line) {
	return Log::Logs.emplace_back(this, severity, line);
}

LogStream& Log::stream(Severity severity, const Section& section, const std::string& key) {
	const auto& value = section.at(key);
	return Log::Logs.emplace_back(this, severity, value.fileIndex, section.name, key, value.value, value.line);
}

LogStream& Log::stream(Severity severity, const std::string& section, const size_t& fileindex, const int& line) {
	return Log::Logs.emplace_back(this, severity, fileindex, section, std::string(), std::string(), line);
}

void Log::output() {
	std::sort(Logs.begin(), Logs.end(), [](const LogStream& l, const LogStream& r) {
		return l.getindex() == r.getindex() ? l.getline() < r.getline() : l.getindex() < r.getindex();
		});
	CanOutput = true;
	Logs.clear();
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

LogStream::LogStream(Log* logger, Severity severity, size_t fileindex, std::string section, std::string key, std::string value, int line)
	: logger(logger), severity(severity), line(line), fileindex(fileindex), section(section), key(key), value(value) {}

LogStream::~LogStream() {
	if (!Log::CanOutput && line != -2)
		return;

    if (severity == Severity::DEFAULT) {
        std::cerr << buffer << std::endl;
        logger->writeLog(buffer);
    }
    else {
        std::ostringstream formattedMessage;
		std::ostringstream plainMessage;

		if (section.empty()) {
			// 控制台输出带颜色的内容
			formattedMessage << logger->getSeverityLabel(severity) << " ";
			if (line >= 0)
				formattedMessage << "第" << line << "行\t| ";
			formattedMessage << buffer;
			std::cerr << formattedMessage.str() << std::endl;

			// 文件写入无颜色内容
			plainMessage << logger->getPlainSeverityLabel(severity) << " ";
			if (line >= 0)
				plainMessage << "第" << line << "行\t| ";
			plainMessage << buffer;
			logger->writeLog(plainMessage.str());
		}
		else {
			auto linenumber = std::format("第{}行", line);
			auto pair = std::format("[{}] ", section);
			auto filename = IniFile::GetFileName(fileindex);

			if (line < 0)
				linenumber = "";

			if (!key.empty())
				pair += std::format("{}={}", key, value);
			size_t blocksize = std::max(filename.size(), linenumber.size());

			formattedMessage << logger->getSeverityLabel(severity) << " " << std::left << std::setw(blocksize) << filename << std::setw(0) << " | " << pair
				<< std::endl << "[详情] " << std::left << std::setw(blocksize) << linenumber << std::setw(0) << " | " << buffer;
			std::cerr << formattedMessage.str() << std::endl;

			plainMessage << logger->getPlainSeverityLabel(severity) << " " << std::left << std::setw(blocksize) << filename << std::setw(0) << " | " << pair
				<< std::endl << "[详情] " << std::left << std::setw(blocksize) << linenumber << std::setw(0) << " | " << buffer;
			logger->writeLog(plainMessage.str());
		}
    }
}
