#pragma once
#include <atomic>
#include <chrono>
#include <map>
#include <mutex>
#include <string>

class ProgressData {
public:
	using time_point = std::chrono::steady_clock::time_point;

	double getPercent() const;
	auto getElapsed() const;

	int line{ 0 };						// 在第几行画
	std::atomic<size_t> processed{ 0 }; // 已处理项
	size_t total{ 0 };                  // 总项数
	std::string name;                   // 进度条名称
	time_point startTime;				// 开始时间
	time_point endTime;					// 完成时间
	bool finished{ false };				// 是否已完成
};

class ProgressBar {
public:
	static ProgressBar INIFileProgress;
	static ProgressBar CheckerProgress;
	static int line;

	ProgressBar() : stopFlag(false), threadStarted(false) {}
	~ProgressBar();

	void addProgressBar(size_t id, const std::string& name, size_t total);
	void updateProgress(size_t id, size_t processed);
	void markFinished(size_t id);
	void stop();

private:
	std::map<size_t, ProgressData> progressBars; // 进度条集合
	std::mutex mutex;
	std::thread displayThread;
	std::atomic<bool> stopFlag;
	bool threadStarted;

	void start();
	void run();
	void loop();
};
