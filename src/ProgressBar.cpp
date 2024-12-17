#include "Helper.h"
#include "ProgressBar.h"
#include <iostream>

Progress::Progress(const std::string& name, size_t total):total(total){
	constexpr size_t fileNameWidth = 25;
	std::cerr << "\033[25h";
	start = string::clamp(name, 25);
	startTime = std::chrono::steady_clock::now();
	draw(false);
}

Progress::~Progress() {
}

void Progress::update() {
	processed++;
	draw();
}

void Progress::stop() {
	processed = total;
	draw();
	std::cerr << "\033[25l";
	std::cerr << "\n";
}

void Progress::draw(bool redraw) {
	if (redraw)
		std::cerr << "\r";

	double percent = getPercent();
	auto elapsed = getElapsed();

	// 固定文件名宽度
	constexpr size_t total = 50;

	// 渲染进度条
	size_t completed = (size_t)(percent / 2);
	size_t remain = total - completed;
	std::cerr << start << std::format("[\033[32m{0:━<{1}}>\033[90m{2:┈<{3}}\033[0m]", "", completed, "", remain);

	// 显示百分比和时间
	std::cerr << std::fixed << std::setprecision(2) << percent << "% (" << elapsed << "ms)";
}

double Progress::getPercent() const {
	return total > 0 ? (double)processed / total * 100 : 0.0;
}

long Progress::getElapsed() const {
	auto time = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(time - startTime).count();
}
