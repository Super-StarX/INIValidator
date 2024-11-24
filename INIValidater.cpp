#include "IniFile.h"
#include "Log.h"
#include "Checker.h"
#include <iostream>
#include <filesystem>
#include <string>

int main(int argc, char* argv[]) {
    try {
        Log log("Checker.log");
        std::string targetFilePath;
        if (argc >= 2)
            targetFilePath = argv[1];
        else {
            LOG << "Enter the path to the target INI file: ";
            std::getline(std::cin, targetFilePath);
        }

		IniFile configIni("INICodingCheck.ini", true);
		IniFile targetIni(targetFilePath, false);;
        Checker checker(configIni, targetIni);
        checker.checkFile();

        LOG << "Validation completed.";
        log.stop();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
