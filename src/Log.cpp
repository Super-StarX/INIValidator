#include "Log.h"
#include <queue>
#include <sstream>

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
	// 共享资源和同步机制
	std::queue<std::string> logQueue;
	std::mutex queueMutex, fileMutex;
	std::condition_variable cv;
	bool stopFlag = false;

	// 打开文件
	std::ofstream logFile(logFileName, std::ios::out | std::ios::trunc);
	if (!logFile.is_open())
		throw std::runtime_error("Unable to open log file: " + logFileName);

	// 写日志线程
	std::thread writerThread([&]() {
		while (true) {
			std::string log;
			{
				std::unique_lock<std::mutex> lock(queueMutex);
				cv.wait(lock, [&]() { return stopFlag || !logQueue.empty(); });

				if (stopFlag && logQueue.empty()) break; // 如果停止标志设定且队列为空，退出线程
				log = logQueue.front();
				logQueue.pop();
			}

			// 写日志到文件（受 fileMutex 保护）
			{
				std::lock_guard<std::mutex> lock(fileMutex);
				logFile << log << std::endl;
			}
		}
	});

	// 将日志放入队列
	for (const auto& log : Logs) {
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			logQueue.push(log.getFileMessage());
		}
		cv.notify_one(); // 通知写线程
	}

	// 打印到控制台
	std::map<std::string, std::map<Severity, int>> fileSeverityCount;
	for (const auto& log : Logs) {
		auto filename = IniFile::GetFileName(log.data.fileindex);
		fileSeverityCount[filename][log.severity]++;
		std::cerr << log.getPrintMessage() << std::endl;
	}
	summary(fileSeverityCount);

	// 设置停止标志并等待线程完成
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		stopFlag = true;
	}
	cv.notify_all(); // 通知线程退出
	writerThread.join();

	// 关闭文件
	logFile.close();
}

void Log::summary(std::map<std::string, std::map<Severity, int>>& fileSeverityCount) {
	std::cerr << "\n======== 日志总结表格 ========" << std::endl;
	std::cerr << "文件\t\t建议\t非法\t错误" << std::endl;
	std::cerr << std::string(40, '-') << std::endl;
	auto getCount = [](const std::map<Severity, int> &map, Severity key) {
		return map.contains(key) ? map.at(key) : 0;
	};
	for (const auto& [filename, severityCount] : fileSeverityCount) {
		std::cerr << filename
			<< "\t" << getCount(severityCount, Severity::INFO)
			<< "\t" << getCount(severityCount, Severity::WARNING)
			<< "\t" << getCount(severityCount, Severity::ERROR) << std::endl;
	}
	std::cerr << "=============================" << std::endl;
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
