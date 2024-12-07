#include "Log.h"
#include "Settings.h"
#include <cstdarg>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <thread>

Log* Log::Instance = nullptr;
std::set<LogStream, LogStream> Log::Logs;

Log::Log() {
	Instance = this;
}

std::string Log::getSeverityLabel(Severity severity) {
	switch (severity) {
	case Severity::DEFAULT: return "";
	case Severity::INFO:    return "\033[32m[建议]\033[0m";
	case Severity::WARNING: return "\033[33m[非法]\033[0m";
	case Severity::ERROR:   return "\033[31m[错误]\033[0m";
	default: __assume(0);
	}
}

std::string Log::getPlainSeverityLabel(Severity severity) {
	switch (severity) {
	case Severity::DEFAULT: return "";
	case Severity::INFO:    return "[建议]";
	case Severity::WARNING: return "[非法]";
	case Severity::ERROR:   return "[错误]";
	default: __assume(0);
	}
}

void Log::output(const std::string& logFileName) {
	std::lock_guard<std::mutex> lock(fileMutex);
	logFile.open(logFileName, std::ios::out | std::ios::trunc);
	if (!logFile.is_open())
		throw std::runtime_error("Unable to open log file: " + logFileName);

	std::thread fileWriter([&]() {
		for (const auto& log : Logs)
			writeLog(log.getFileMessage());
	});

	// 主线程负责屏幕输出
	for (const auto& log : Logs)
		std::cerr << log.getFileMessage() << std::endl;

	// 等待文件写入完成
	fileWriter.join();
	std::lock_guard<std::mutex> lock(logMutex);
	Logs.clear();
	std::lock_guard<std::mutex> lock(fileMutex);
	logFile.close();
}

void Log::writeLog(const std::string& logLine) {
	std::lock_guard<std::mutex> lock(fileMutex);
	if (logFile.is_open())
		logFile << logLine << std::endl;
}

LogStream::LogStream(Log* logger, Severity severity, int line, ...)
	: logger(logger), severity(severity), line(line) {
	va_list args;
	va_start(args, Settings::Instance->recordKeyNotExist);
	buffer = formatString(Settings::Instance->recordKeyNotExist.c_str(), args);  // 调用格式化函数
	va_end(args);
}

LogStream::LogStream(Log* logger, Severity severity, size_t fileindex, 
	std::string section, std::string key, std::string value, int line, ...)
	: logger(logger), severity(severity), line(line), fileindex(fileindex), 
	section(section), key(key), value(value) {
	va_list args;
	va_start(args, Settings::Instance->recordKeyNotExist);
	buffer = formatString(Settings::Instance->recordKeyNotExist.c_str(), args);  // 调用格式化函数
	va_end(args);
}

LogStream::~LogStream() {}

std::string LogStream::getFileMessage() const {
	std::ostringstream plainMessage;
	plainMessage << Log::getPlainSeverityLabel(severity) << " ";
	if (line >= 0)
		plainMessage << "第" << line << "行\t| ";
	return plainMessage.str();
}

std::string LogStream::getPrintMessage() const {
	std::ostringstream formattedMessage;
	formattedMessage << Log::getSeverityLabel(severity) << " ";
	if (line >= 0)
		formattedMessage << "第" << line << "行\t| ";

	return formattedMessage.str();
}

bool LogStream::operator()(const LogStream& l, const LogStream& r) {
	return l.getindex() == r.getindex() ? l.getline() < r.getline() : l.getindex() < r.getindex();
}

std::string LogStream::formatString(const char* format, va_list args) {
	va_list argsCopy;
	va_copy(argsCopy, args);
	int size = std::vsnprintf(nullptr, 0, format, argsCopy) + 1; // 包括空终止符
	va_end(argsCopy);

	if (size <= 0)
		throw std::runtime_error("Error formatting string");

	std::vector<char> buffer(size);
	std::vsnprintf(buffer.data(), size, format, args);
	return std::string(buffer.data(), buffer.size() - 1); // 去掉空终止符
}