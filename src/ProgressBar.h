#pragma once
#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <map>
#include <atomic>
#include <chrono>
#include <string>
#include <sstream>

struct ProgressData {
	std::atomic<size_t> processed{ 0 };   // 已处理项
	size_t total{ 0 };                    // 总项数
	std::string name;                   // 进度条名称
	std::chrono::steady_clock::time_point startTime; // 开始时间
	std::chrono::steady_clock::time_point endTime; // 完成时间
	bool finished{ false };               // 是否已完成
};

class ProgressBar {
public:
	static ProgressBar INIFileProgress;
	static ProgressBar CheckerProgress;

	ProgressBar() : stopFlag(false), threadStarted(false) {}

	~ProgressBar() {
		stop(); // 确保析构时停止线程
	}

	void addProgressBar(int id, const std::string& name, size_t total) {
		std::lock_guard<std::mutex> lock(mutex);
		progressBars[id].total = total;
		progressBars[id].name = name;
		progressBars[id].startTime = std::chrono::steady_clock::now();
		start(); // 启动显示线程
	}

	void updateProgress(int id, size_t processed) {
		std::lock_guard<std::mutex> lock(mutex);
		if (progressBars.count(id))
			progressBars[id].processed = processed;
	}

	void markFinished(int id) {
		std::lock_guard<std::mutex> lock(mutex);
		if (progressBars.count(id)) {
			progressBars[id].endTime = std::chrono::steady_clock::now();
			progressBars[id].finished = true;
			progressBars[id].processed = progressBars[id].total;
		}
	}

	void stop() {
		if (threadStarted) {
			std::cout << "\033[?25h";
			stopFlag = true;
			if (displayThread.joinable())
				displayThread.join();
		}
	}

private:
	std::map<int, ProgressData> progressBars; // 进度条集合
	std::mutex mutex;
	std::thread displayThread;
	std::atomic<bool> stopFlag;
	bool threadStarted;

	void start() {
		if (!threadStarted) {
			std::cout << "\033[?25l";
			threadStarted = true;
			displayThread = std::thread(&ProgressBar::run, this);
		}
	}

	void run() {
		while (!stopFlag) {
			{
				std::lock_guard<std::mutex> lock(mutex);
				int line = 0;
				for (const auto& [id, progress] : progressBars) {
					double percent = progress.total > 0
						? (double)progress.processed / progress.total * 100
						: 0.0;
					auto endTime = progressBars[id].finished ? progressBars[id].endTime : std::chrono::steady_clock::now();
					auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - progress.startTime).count();

					// 固定文件名宽度
					constexpr size_t total = 50;
					constexpr size_t fileNameWidth = 20;
					auto name = progress.name;
					if (name.size() > fileNameWidth)
						name = name.substr(0, fileNameWidth - 3) + "..."; // 超出部分用省略号
					else
						name += std::string(fileNameWidth - name.size(), ' '); // 补齐空格

					std::cout << "\033[" << ++line << ";0H"; // 定位光标到行首

					// 渲染进度条
					size_t completed = (size_t)(percent / 2);
					size_t remain = total - completed;
					std::cout << name << std::format("\033[32m{0:━<{1}}\033[90m{2:┈<{3}}\033[0m", "", completed, "", remain);

					// 显示百分比和时间
					std::cout << std::fixed << std::setprecision(2) << percent << "% ("
						<< elapsed << "ms)\n";
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
};
