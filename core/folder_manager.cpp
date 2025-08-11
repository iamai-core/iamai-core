#include "folder_manager.h"
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#include <stdexcept>
#include <windows.h>
#include <shlobj.h>
#include <iostream>

namespace iamai {

namespace fs = std::filesystem;

FolderManager& FolderManager::getInstance() {
    static FolderManager instance;
    return instance;
}

std::string FolderManager::getWindowsFolder(int folderId) const {
    if (folderId == CSIDL_LOCAL_APPDATA) {
        wchar_t path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, folderId, NULL, SHGFP_TYPE_CURRENT, path))) {
            std::wstring wPath(path);
            return std::string(wPath.begin(), wPath.end());
        }
    }
    else if (folderId == CSIDL_PERSONAL) {
        char* userProfile = nullptr;
        size_t len;
        _dupenv_s(&userProfile, &len, "USERPROFILE");
        if (userProfile != nullptr) {
            std::string documentsPath = std::string(userProfile) + "\\Documents";
            free(userProfile);
            return documentsPath;
        }
    }
    
    return "";
}

bool FolderManager::createFolderStructure() {
    try {
        std::cout << "Starting folder structure creation..." << std::endl;
        // Get system paths
        std::string localAppData = getWindowsFolder(CSIDL_LOCAL_APPDATA);
        std::string documentsPath = getWindowsFolder(CSIDL_PERSONAL);
        
        std::cout << "Got AppData path: " << localAppData << std::endl;
        std::cout << "Got Documents path: " << documentsPath << std::endl;
        
        if (localAppData.empty() || documentsPath.empty()) {
            throw std::runtime_error("Failed to get system folders paths");
        }

        // Set up paths
        m_appDataPath = fs::path(localAppData) / "iamai";
        m_binPath = m_appDataPath / "bin";
        m_documentsPath = fs::path(documentsPath) / "iamai";
        m_modelsPath = m_documentsPath / "models";

        // Create all directories
        std::cout << "Creating folder structure..." << std::endl;
        
        // Create AppData folders
        if (!fs::exists(m_appDataPath)) {
            fs::create_directories(m_appDataPath);
            std::cout << "Created: " << m_appDataPath << std::endl;
        }
        if (!fs::exists(m_binPath)) {
            fs::create_directories(m_binPath);
            std::cout << "Created: " << m_binPath << std::endl;
        }
        
        // Create Documents folders
        if (!fs::exists(m_documentsPath)) {
            fs::create_directories(m_documentsPath);
            std::cout << "Created: " << m_documentsPath << std::endl;
        }
        if (!fs::exists(m_modelsPath)) {
            fs::create_directories(m_modelsPath);
            std::cout << "Created: " << m_modelsPath << std::endl;
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error creating folder structure: " << e.what() << std::endl;
        return false;
    }
}

fs::path FolderManager::getAppDataPath() const {
    return m_appDataPath;
}

fs::path FolderManager::getBinPath() const {
    return m_binPath;
}

fs::path FolderManager::getDocumentsPath() const {
    return m_documentsPath;
}

fs::path FolderManager::getModelsPath() const {
    return m_modelsPath;
}

} // namespace iamai