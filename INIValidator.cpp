#include "Checker.h"
#include "Helper.h"
#include "IniFile.h"
#include "Log.h"
#include "Settings.h"
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <windows.h>

static void loadFromInput(IniFile& targetIni) {
	std::string targetFilePath;
	std::cout << "请输入要检查的INI文件路径: ";
	std::getline(std::cin, targetFilePath);
	targetFilePath = std::regex_replace(targetFilePath, std::regex("^\"|\"$"), "");
	if (!targetFilePath.empty()) {
		std::filesystem::path targetPath(targetFilePath);
		if (std::filesystem::is_regular_file(targetPath))
			targetIni.load(targetFilePath);
		else if (std::filesystem::is_directory(targetPath)) {
			for (const auto& filePath : targetFilePath)
				targetIni.load(std::to_string(filePath));
		}
		else {
			std::cerr << "输入路径无效，无法加载文件: " << targetFilePath << std::endl;
			loadFromInput(targetIni);
		}
	}
	else if (!Settings::Instance->folderPath.empty()) {
		// 用户直接按回车，使用默认目录
		std::string defaultDir = Settings::Instance->folderPath;
		for (const auto& entry : std::filesystem::directory_iterator(defaultDir)) {
			if (entry.is_regular_file() && entry.path().extension() == ".ini")
				targetIni.load(entry.path().string());
		}
	}
	else {
		std::cerr << "默认路径为空，无法加载文件。" << std::endl;
		loadFromInput(targetIni);
	}
}

static void loadFromArg(int argc, char* argv[], IniFile& targetIni) {
	for (int i = 1; i < argc; ++i) {
		std::filesystem::path path(argv[i]);
		if (std::filesystem::is_regular_file(path))
			targetIni.load(path.string());
		else if (std::filesystem::is_directory(path)) {
			std::vector<std::string> filePaths;
			for (const auto& entry : std::filesystem::recursive_directory_iterator(path.string()))
				if (entry.is_regular_file() && entry.path().extension() == ".ini")
					targetIni.load(entry.path().string());
		}
		else {
			std::cerr << "错误路径: " << path.string();
			loadFromInput(targetIni);
		}
	}
}

int main(int argc, char* argv[]) {
    try {
		SetConsoleOutputCP(CP_UTF8);
		system("title INI Validator");
#ifndef _DEBUG
		std::filesystem::path exe_path = std::filesystem::path(argv[0]).parent_path();
		std::filesystem::current_path(exe_path);
#endif // !_DEBUG

        auto log = Log();
		Settings setting(IniFile("Settings.ini", true));
		IniFile configIni("INICodingCheck.ini", true);

		IniFile targetIni;
		if (argc >= 2)
			loadFromArg(argc, argv, targetIni);
		else
			loadFromInput(targetIni);
		SetConsoleCP(CP_UTF8);

        Checker checker(configIni, targetIni);
        checker.checkFile();

		log.output("Checker.log");
		std::cout << "\n检查完毕" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

	system("pause>nul");

    return 0;
}
