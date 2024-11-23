#include "IniFile.h"
#include "ConfigChecker.h"
#include <iostream>
#include <filesystem>
#include <string>

int main(int argc, char* argv[]) {
    try {
        std::string targetFilePath;

        // 检查命令行参数
        if (argc >= 2) {
            targetFilePath = argv[1];
        }
        else {
            std::cout << "No file provided. Please drag a file onto the program or type the path manually.\n";
            std::cout << "Enter the path to the target INI file: ";
            std::getline(std::cin, targetFilePath);

            // 检查是否输入为空
            if (targetFilePath.empty()) {
                std::cerr << "Error: No file path provided." << std::endl;
                return 1;
            }
        }

        // 加载配置文件
        IniFile configIni("INICodingCheck.ini");

        // 加载目标 INI 文件
        IniFile targetIni(targetFilePath);

        // 创建并运行配置检查器
        ConfigChecker checker(configIni);
        checker.checkFile(targetIni);

        std::cout << "Validation completed." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
