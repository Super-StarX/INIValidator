#pragma once
#include <atomic>
#include <chrono>
#include <map>
#include <mutex>
#include <string>

class Progress {
public:
	Progress(const std::string& name, size_t total);
	~Progress();
	void update();
	void stop();
private:
	using time_point = std::chrono::steady_clock::time_point;

	void draw(bool redraw = true);
	double getPercent() const;
	long getElapsed() const;

	std::atomic<size_t> processed{ 0 }; // 已处理项
	size_t total{ 0 };                  // 总项数
	std::string start;                  // 进度条名称
	time_point startTime;				// 开始时间
	time_point endTime;					// 完成时间
};