#pragma once

#include <string>
#include <filesystem>

namespace iamai {

// Cross-platform folder IDs
#define CSIDL_LOCAL_APPDATA 0  // AppData or ~/Library/Application Support
#define CSIDL_PERSONAL 1       // Documents folder

class FolderManager {
public:
    static FolderManager& getInstance();
    
    bool createFolderStructure();
    
    std::filesystem::path getAppDataPath() const;
    std::filesystem::path getBinPath() const;
    std::filesystem::path getDocumentsPath() const;
    std::filesystem::path getModelsPath() const;
    
private:
    FolderManager() = default;
    ~FolderManager() = default;
    FolderManager(const FolderManager&) = delete;
    FolderManager& operator=(const FolderManager&) = delete;
    
    // Platform-specific folder getters
    std::string getSystemFolder(int folderId) const;
    
    std::filesystem::path m_appDataPath;
    std::filesystem::path m_binPath;
    std::filesystem::path m_documentsPath;
    std::filesystem::path m_modelsPath;
};

} // namespace iamai
