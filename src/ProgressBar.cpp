#include "ProgressBar.h"

ProgressBar ProgressBar::INIFileProgress;
ProgressBar ProgressBar::CheckerProgress;
int ProgressBar::line = 2;

double ProgressData::getPercent() const {
	return total > 0 ? (double)processed / total * 100 : 0.0;
}

auto ProgressData::getElapsed() const {
	auto time = finished ? endTime : std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(time - startTime).count();
}

ProgressBar::~ProgressBar() {
	stop(); // 确保析构时停止线程
}

void ProgressBar::addProgressBar(int id, const std::string& name, size_t total) {
	std::lock_guard<std::mutex> lock(mutex);
	progressBars[id].total = total;
	progressBars[id].line = line++;
	progressBars[id].name = name;
	progressBars[id].startTime = std::chrono::steady_clock::now();
	start(); // 启动显示线程
}

void ProgressBar::updateProgress(int id, size_t processed) {
	std::lock_guard<std::mutex> lock(mutex);
	if (progressBars.count(id))
		progressBars[id].processed = processed;
}

void ProgressBar::markFinished(int id) {
	std::lock_guard<std::mutex> lock(mutex);
	if (progressBars.count(id)) {
		progressBars[id].endTime = std::chrono::steady_clock::now();
		progressBars[id].finished = true;
		progressBars[id].processed = progressBars[id].total;
	}
}

void ProgressBar::stop() {
	if (threadStarted) {
		run();
		std::cout << "\033[?25h";
		stopFlag = true;
		if (displayThread.joinable())
			displayThread.join();
	}
}

void ProgressBar::start() {
	if (!threadStarted) {
		std::cout << "\033[?25l";
		threadStarted = true;
		displayThread = std::thread(&ProgressBar::loop, this);
	}
}

void ProgressBar::run() {
	std::lock_guard<std::mutex> lock(mutex);
	for (const auto& [id, progress] : progressBars) {
		double percent = progress.getPercent();
		auto elapsed = progress.getElapsed();

		// 固定文件名宽度
		constexpr size_t total = 50;
		constexpr size_t fileNameWidth = 25;
		std::string name = progress.name;
		if (name.size() > fileNameWidth)
			name = name.substr(0, fileNameWidth - 3) + "..."; // 超出部分用省略号
		else
			name += std::string(fileNameWidth - name.size(), ' '); // 补齐空格

		std::cout << "\033[" << progress.line << ";0H"; // 定位光标到行首

		// 渲染进度条
		size_t completed = (size_t)(percent / 2);
		size_t remain = total - completed;
		std::cout << name << std::format("[\033[32m{0:━<{1}}\033[91m{2:┈<{3}}\033[0m]", "", completed, "", remain);

		// 显示百分比和时间
		std::cout << std::fixed << std::setprecision(2) << percent << "% (" << elapsed << "ms)\n";
	}
}

void ProgressBar::loop() {
	while (!stopFlag) {
		run();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
