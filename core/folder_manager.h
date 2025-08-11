#pragma once

#include <string>
#include <filesystem>

namespace iamai {

class FolderManager {
public:
    // Get singleton instance
    static FolderManager& getInstance();

    // Delete copy constructor and assignment operator
    FolderManager(const FolderManager&) = delete;
    FolderManager& operator=(const FolderManager&) = delete;

    // Initialize folder structure
    bool createFolderStructure();

    // Get paths for persistent data (survives app updates)
    std::filesystem::path getConfigPath() const;      // Settings/INI files
    std::filesystem::path getDataPath() const;        // Database files, logs
    std::filesystem::path getModelsPath() const;      // Model files
    std::filesystem::path getCachePath() const;       // Temporary/cache files

    // Get paths for application binaries (may be updated/replaced)
    std::filesystem::path getAppBinaryPath() const;   // Application executable location
    std::filesystem::path getRuntimeLibsPath() const; // CUDA DLLs, runtime libraries

private:
    // Private constructor for singleton
    FolderManager() = default;

    // Helper functions
    std::filesystem::path getSystemConfigPath() const;    // ~/.config, %APPDATA%
    std::filesystem::path getSystemDataPath() const;      // ~/.local/share, %LOCALAPPDATA%
    std::filesystem::path getSystemCachePath() const;     // ~/.cache, %TEMP%
    std::filesystem::path getSystemDocumentsPath() const; // ~/Documents, %USERPROFILE%\Documents
    std::filesystem::path getCurrentExecutablePath() const;

    // Cached paths
    mutable std::filesystem::path m_configPath;
    mutable std::filesystem::path m_dataPath;
    mutable std::filesystem::path m_modelsPath;
    mutable std::filesystem::path m_cachePath;
    mutable std::filesystem::path m_appBinaryPath;
    mutable std::filesystem::path m_runtimeLibsPath;
};

} // namespace iamai
