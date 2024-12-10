#include "Checker.h"
#include "IniFile.h"
#include "Log.h"
#include "Settings.h"
#include <filesystem>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        auto log = Log();
        std::string targetFilePath;
        if (argc >= 2)
            targetFilePath = argv[1];
        else {
            std::cout << "请输入要检查的INI文件路径: ";
            std::getline(std::cin, targetFilePath);
        }

		Settings setting(IniFile("Settings.ini", false));
		IniFile configIni("INICodingCheck.ini", true);
		IniFile targetIni(targetFilePath, false);
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
