#include "Checker.h"
#include "Helper.h"
#include "IniFile.h"
#include "Log.h"
#include "Settings.h"
#include <filesystem>
#include <iostream>
#include <string>
#include <windows.h>

int main(int argc, char* argv[]) {
    try {
		SetConsoleOutputCP(CP_UTF8);
		system("title INI Validator");
#ifndef _DEBUG
		std::filesystem::path exe_path = std::filesystem::path(argv[0]).parent_path();
		std::filesystem::current_path(exe_path);
#endif // !_DEBUG

        auto log = Log();
        std::string targetFilePath;
        if (argc >= 2)
			targetFilePath = argv[1];
        else {
            std::cout << "请输入要检查的INI文件路径: ";
            std::getline(std::cin, targetFilePath);
        }
		SetConsoleCP(CP_UTF8);

		Settings setting(IniFile("Settings.ini", true));
		IniFile configIni("INICodingCheck.ini", true);
		IniFile targetIni(targetFilePath);
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
