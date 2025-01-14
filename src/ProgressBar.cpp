#include "ProgressBar.h"
#include "Helper.h"

constexpr size_t fileNameWidth = 25; // 文件名宽度
constexpr size_t totalLength = 50; // 进度条宽度

Progress::Progress() {}

Progress::~Progress() {
	stopDrawing();
}

Progress& Progress::instance() {
	static Progress instance;
	return instance;
}

void Progress::start(const std::string& name, size_t total) {
	instance()._start(name, total);
}

void Progress::update() {
	instance()._update();
}

void Progress::stop() {
	instance()._stop();
}

void Progress::_start(const std::string& name, size_t total) {
	stop(); // 确保上一个进度条结束

	std::lock_guard<std::mutex> lock(mtx);
	this->startName = string::clamp(name, fileNameWidth);
	this->total = total;
	this->processed = 0;
	this->startTime = std::chrono::steady_clock::now();
	stopFlag = false;

	// 启动渲染线程，每 25ms 刷新一次
	drawThread = std::thread([this]() {
		while (!stopFlag) {
			draw();
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
		}
	});
}

void Progress::_update() {
	++processed;
}

void Progress::_stop() {
	if (!stopFlag) {
		processed = total;
		stopDrawing();
		draw();
		std::cerr << "\n"; // 结束时换行
	}
}

void Progress::stopDrawing() {
	if (drawThread.joinable()) {
		stopFlag = true;
		drawThread.join();
	}
}

void Progress::draw() {
	double percent = getPercent();
	auto elapsed = getElapsed();

	// 渲染进度条
	size_t completed = (size_t)(percent / 2);
	size_t remain = totalLength - completed;
	std::cerr << "\r" << startName << std::format("[\033[32m{0:━<{1}}>\033[90m{2:┈<{3}}\033[0m]", "", completed, "", remain);

	// 显示百分比和时间
	std::cerr << std::fixed << std::setprecision(2) << processed << "/" << total << " (" << percent << "% " << elapsed << "ms)";
}

double Progress::getPercent() const {
	return total > 0 ? std::clamp((double)processed / total * 100, 0.0, 100.0) : 0.0;
}

long long Progress::getElapsed() const {
	auto now = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
}

// 封装for循环：对容器的每个元素调用func
template <typename Container, typename Func>
void Progress::forEach(const std::string& name, const Container& container, Func func) {
	start(name, container.size());
	for (const auto& [key, value] : container) {
		func(key, value);
		update();
	}
	stop();
}