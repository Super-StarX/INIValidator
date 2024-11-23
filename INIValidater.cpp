#include "IniFile.h"
#include "Log.h"
#include "Checker.h"
#include <iostream>
#include <filesystem>
#include <string>

int main(int argc, char* argv[]) {
    try {
        std::string targetFilePath;
        if (argc >= 2)
            targetFilePath = argv[1];
        else {
            std::cout << "Enter the path to the target INI file: ";
            std::getline(std::cin, targetFilePath);
        }

        Log logger("Checker.log");
        IniFile configIni("INICodingCheck.ini");
        IniFile targetIni(targetFilePath);

        Checker checker(configIni);
        checker.checkFile(targetIni);

        std::cout << "Validation completed." << std::endl;
        logger.stop();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
