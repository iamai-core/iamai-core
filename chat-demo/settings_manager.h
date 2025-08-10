#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include "imgui.h"
#include "imgui_internal.h"
#include <string>
#include <unordered_map>

class SettingsManager {
public:
    SettingsManager(const std::string& typeName);
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
    std::string typeName;
    ImGuiID typeHash;
    std::unordered_map<std::string, std::string> settings;

    static void* ReadOpenFn(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name);
    static void ReadLineFn(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line);
    static void WriteAllFn(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf);

    void* readOpen(const char* name);
    void readLine(void* entry, const char* line);
    void writeAll(ImGuiTextBuffer* buf);
};

#endif // SETTINGS_MANAGER_H
