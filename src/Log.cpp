#include "Log.h"
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <thread>

Log* Log::Instance = nullptr;
std::set<LogStream> Log::Logs;

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

	// 写日志线程
	std::thread fileWriter([&]() {
		for (const auto& log : Logs)
			writeLog(log.getFileMessage());
	});

	for (const auto& log : Logs)
		std::cerr << log.getPrintMessage() << std::endl;

	// 等待文件写入完成
	fileWriter.join();
	Logs.clear();
	logFile.close();
}

void Log::writeLog(const std::string& logLine) {
	std::lock_guard<std::mutex> lock(fileMutex);
	if (logFile.is_open())
		logFile << logLine << std::endl;
}

LogStream::LogStream(Severity severity, const LogData& logdata, std::string buffer)
	: severity(severity), data(logdata), buffer(buffer) {}

std::string LogStream::getFileMessage() const {
	std::ostringstream plainMessage;
	plainMessage << Log::getPlainSeverityLabel(severity) << " " << generateLogMessage();
	return plainMessage.str();
}

std::string LogStream::getPrintMessage() const {
	std::ostringstream formattedMessage;
	formattedMessage << Log::getSeverityLabel(severity) << " " << generateLogMessage();
	return formattedMessage.str();
}

std::string LogStream::generateLogMessage() const {
	std::string retval;

	if (data.section.empty()) {
		std::string line;

		if (data.line >= 0)
			line = std::format("第{}行\t| ", data.line);

		retval = std::format("{}{}", line, buffer);
	}
	else {
		auto linenumber = std::format("第{}行", data.line);
		auto pair = std::format("[{}] ", data.section);
		auto filename = IniFile::GetFileName(data.fileindex);

		if (data.line < 0)
			linenumber = "";

		if (!data.key.empty())
			pair += std::format("{}={}", data.key, data.value);
		size_t blocksize = std::max(filename.size(), linenumber.size());

		// 一共6个参数，其中1号和4号是blocksize（对齐大小），作为变参传入
		// 0号和3号参数中，:代表其有填充需求，<代表左对齐，后边的数字是填充大小，即blocksize
		retval = std::format("{0:<{1}} | {2}\n[详情] {3:<{4}} | {5}", filename, blocksize, pair, linenumber, blocksize, buffer);
	}

	return retval;
}
