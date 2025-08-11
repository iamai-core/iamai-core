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

    // Get paths
    std::filesystem::path getAppDataPath() const;
    std::filesystem::path getBinPath() const;
    std::filesystem::path getDocumentsPath() const;
    std::filesystem::path getModelsPath() const;

private:
    // Private constructor for singleton
    FolderManager() = default;

    // Helper functions
    std::filesystem::path getSystemAppDataPath() const;
    std::filesystem::path getSystemDocumentsPath() const;

    // Cached paths
    mutable std::filesystem::path m_appDataPath;
    mutable std::filesystem::path m_binPath;
    mutable std::filesystem::path m_documentsPath;
    mutable std::filesystem::path m_modelsPath;
};

} // namespace iamai
