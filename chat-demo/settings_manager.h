#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <string>
#include <unordered_map>
#include <filesystem>

class SettingsManager {
public:
    SettingsManager(const std::filesystem::path& iniFilePath);
    ~SettingsManager();

    void setInt(const std::string& key, int value);
    void setFloat(const std::string& key, float value);
    void setBool(const std::string& key, bool value);
    void setString(const std::string& key, const std::string& value);

    int getInt(const std::string& key, int defaultValue = 0);
    float getFloat(const std::string& key, float defaultValue = 0.0f);
    bool getBool(const std::string& key, bool defaultValue = false);
    std::string getString(const std::string& key, const std::string& defaultValue = "");

private:
    std::filesystem::path iniFilePath;
    std::unordered_map<std::string, std::string> settings;

    void loadFromFile();
    void saveToFile();
    std::string trim(const std::string& str);
};

#endif // SETTINGS_MANAGER_H
