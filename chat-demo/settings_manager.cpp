#include "settings_manager.h"
#include "imgui_internal.h"
#include <sstream>

SettingsManager::SettingsManager(const std::string& typeName) : typeName(typeName) {
    typeHash = ImHashStr(typeName.c_str());

    ImGuiSettingsHandler handler = {};
    handler.TypeName = this->typeName.c_str();
    handler.TypeHash = typeHash;
    handler.UserData = this;
    handler.ReadOpenFn = ReadOpenFn;
    handler.ReadLineFn = ReadLineFn;
    handler.WriteAllFn = WriteAllFn;

    ImGui::AddSettingsHandler(&handler);
}

SettingsManager::~SettingsManager() {
    // Only remove handler if ImGui context still exists
    if (ImGui::GetCurrentContext() != nullptr) {
        ImGui::RemoveSettingsHandler(typeName.c_str());
    }
}

void SettingsManager::setInt(const std::string& key, int value) {
    settings[key] = std::to_string(value);
}

void SettingsManager::setFloat(const std::string& key, float value) {
    settings[key] = std::to_string(value);
}

void SettingsManager::setBool(const std::string& key, bool value) {
    settings[key] = value ? "1" : "0";
}

void SettingsManager::setString(const std::string& key, const std::string& value) {
    settings[key] = value;
}

int SettingsManager::getInt(const std::string& key, int defaultValue) {
    auto it = settings.find(key);
    if (it != settings.end()) {
        return std::stoi(it->second);
    }
    return defaultValue;
}

float SettingsManager::getFloat(const std::string& key, float defaultValue) {
    auto it = settings.find(key);
    if (it != settings.end()) {
        return std::stof(it->second);
    }
    return defaultValue;
}

bool SettingsManager::getBool(const std::string& key, bool defaultValue) {
    auto it = settings.find(key);
    if (it != settings.end()) {
        return it->second == "1";
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

void* SettingsManager::ReadOpenFn(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name) {
    SettingsManager* manager = static_cast<SettingsManager*>(handler->UserData);
    return manager->readOpen(name);
}

void SettingsManager::ReadLineFn(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line) {
    SettingsManager* manager = static_cast<SettingsManager*>(handler->UserData);
    manager->readLine(entry, line);
}

void SettingsManager::WriteAllFn(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf) {
    SettingsManager* manager = static_cast<SettingsManager*>(handler->UserData);
    manager->writeAll(buf);
}

void* SettingsManager::readOpen(const char* name) {
    return this;
}

void SettingsManager::readLine(void* entry, const char* line) {
    std::string lineStr(line);
    size_t equalPos = lineStr.find('=');
    if (equalPos != std::string::npos) {
        std::string key = lineStr.substr(0, equalPos);
        std::string value = lineStr.substr(equalPos + 1);
        settings[key] = value;
    }
}

void SettingsManager::writeAll(ImGuiTextBuffer* buf) {
    if (settings.empty()) return;

    buf->appendf("[%s][Data]\n", typeName.c_str());
    for (const auto& pair : settings) {
        buf->appendf("%s=%s\n", pair.first.c_str(), pair.second.c_str());
    }
    buf->append("\n");
}
