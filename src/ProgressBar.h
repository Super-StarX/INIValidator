#pragma once
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <iostream>

class Progress {
public:
	static Progress& instance();  // 单实例获取

	static void start(const std::string& name, size_t total);
	static void update();
	static void stop();

	~Progress();

	template <typename Container, typename Func>
	void forEach(const std::string& name, const Container& container, Func func);

private:
	Progress(); // 私有构造函数
	Progress(const Progress&) = delete;
	Progress& operator=(const Progress&) = delete;

	void _start(const std::string& name, size_t total);
	void _update();
	void _stop();
	void draw();         // 渲染进度条
	void stopDrawing();  // 停止刷新线程
	double getPercent() const;
	long long getElapsed() const;

	std::string startName;           // 进度条名称
	size_t total{ 0 };               // 总项数
	std::atomic<size_t> processed{ 0 }; // 已处理项
	std::chrono::steady_clock::time_point startTime;

	std::mutex mtx;
	std::atomic<bool> stopFlag{ true };
	std::thread drawThread;  // 渲染线程
};
