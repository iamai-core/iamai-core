#include "settings_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>

SettingsManager::SettingsManager(const std::filesystem::path& iniFilePath) : iniFilePath(iniFilePath) {
    // Load existing settings
    loadFromFile();
}

SettingsManager::~SettingsManager() {
    // Save settings on destruction
    saveToFile();
}

std::string SettingsManager::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void SettingsManager::loadFromFile() {
    std::ifstream file(iniFilePath);
    if (!file.is_open()) {
        std::cout << "Settings file not found, will create new one: " << iniFilePath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // Parse key=value pairs
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = trim(line.substr(0, equalPos));
            std::string value = trim(line.substr(equalPos + 1));
            settings[key] = value;
        }
    }

    file.close();
    std::cout << "Loaded " << settings.size() << " settings from: " << iniFilePath << std::endl;
}

void SettingsManager::saveToFile() {
    // Ensure directory exists
    std::filesystem::create_directories(iniFilePath.parent_path());

    std::ofstream file(iniFilePath);
    if (!file.is_open()) {
        std::cerr << "Failed to save settings to: " << iniFilePath << std::endl;
        return;
    }

    // Write header
    file << "# " << iniFilePath.filename().string() << "\n";
    file << "# This file is automatically managed\n\n";

    // Write all settings
    for (const auto& pair : settings) {
        file << pair.first << "=" << pair.second << "\n";
    }

    file.close();
}

void SettingsManager::setInt(const std::string& key, int value) {
    settings[key] = std::to_string(value);
    saveToFile();
}

void SettingsManager::setFloat(const std::string& key, float value) {
    settings[key] = std::to_string(value);
    saveToFile();
}

void SettingsManager::setBool(const std::string& key, bool value) {
    settings[key] = value ? "1" : "0";
    saveToFile();
}

void SettingsManager::setString(const std::string& key, const std::string& value) {
    settings[key] = value;
    saveToFile();
}

int SettingsManager::getInt(const std::string& key, int defaultValue) {
    auto it = settings.find(key);
    if (it != settings.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

float SettingsManager::getFloat(const std::string& key, float defaultValue) {
    auto it = settings.find(key);
    if (it != settings.end()) {
        try {
            return std::stof(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

bool SettingsManager::getBool(const std::string& key, bool defaultValue) {
    auto it = settings.find(key);
    if (it != settings.end()) {
        return it->second == "1" || it->second == "true";
    }
    return defaultValue;
}

std::string SettingsManager::getString(const std::string& key, const std::string& defaultValue) {
    auto it = settings.find(key);
    if (it != settings.end()) {
        return it->second;
    }
    return defaultValue;
}
