#include "IniFile.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <algorithm>
#include <filesystem>

IniFile::IniFile(const std::string& filepath) {
    load(filepath);
}

void IniFile::load(const std::string& filepath) {
    if (!std::filesystem::exists(filepath)) {
        throw std::runtime_error("File not found: " + filepath);
    }
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    std::string line, currentSection;
    while (std::getline(file, line)) {
        line = trim(removeInlineComment(line));
        if (line.empty()) continue;

        // ¼ì²âÐÂ Section
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
        }
        else if (!currentSection.empty()) {
            auto delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = trim(line.substr(0, delimiterPos));
                std::string value = trim(line.substr(delimiterPos + 1));
                sections[currentSection][key] = value;
            }
            else {
                sections[currentSection][line] = ""; // µ¥¶ÀµÄ¼ü
            }
        }
    }
    processIncludes(std::filesystem::path(filepath).parent_path().string());
    processInheritance();
}

void IniFile::processIncludes(const std::string& basePath) {
    if (sections.count("#include")) {
        for (const auto& [key, value] : sections["#include"]) {
            IniFile includedFile(basePath + "/" + value);
            for (const auto& [sec, keys] : includedFile.sections) {
                sections[sec].insert(keys.begin(), keys.end());
            }
        }
    }
}

void IniFile::processInheritance() {
    std::vector<std::string> sectionsToProcess;
    for (const auto& [section, keys] : sections) {
        sectionsToProcess.push_back(section);
    }

    for (const std::string& sectionName : sectionsToProcess) {
        auto pos = sectionName.find(':');
        if (pos != std::string::npos) {
            std::string section = sectionName.substr(0, pos);
            std::string parent = sectionName.substr(pos + 1);

            if (sections.count(parent)) {
                sections[section].insert(sections[parent].begin(), sections[parent].end());
            }
        }
    }
}

std::string IniFile::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

std::string IniFile::removeInlineComment(const std::string& str) {
    size_t commentPos = str.find(';');
    return commentPos != std::string::npos ? str.substr(0, commentPos) : str;
}
