#include "folder_manager.h"
#include <stdexcept>
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
#else
    #include <pwd.h>
    #include <unistd.h>
#endif

namespace iamai {

namespace fs = std::filesystem;

FolderManager& FolderManager::getInstance() {
    static FolderManager instance;
    return instance;
}

#ifdef _WIN32
// Windows implementation
std::string FolderManager::getSystemFolder(int folderId) const {
    CHAR path[MAX_PATH];
    int csidlFlag;
    
    switch (folderId) {
        case CSIDL_LOCAL_APPDATA:
            csidlFlag = CSIDL_LOCAL_APPDATA;
            break;
        case CSIDL_PERSONAL:
            csidlFlag = CSIDL_PERSONAL;
            break;
        default:
            return "";
    }
    
    if (SUCCEEDED(SHGetFolderPathA(NULL, csidlFlag, NULL, 0, path))) {
        return std::string(path);
    }
    
    return "";
}
#else
// macOS/Unix implementation
std::string FolderManager::getSystemFolder(int folderId) const {
    // Get user's home directory
    const char* homeDir = getenv("HOME");
    
    if (!homeDir) {
        // Fallback if HOME is not set
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) {
            homeDir = pwd->pw_dir;
        } else {
            return "";
        }
    }
    
    if (folderId == CSIDL_LOCAL_APPDATA) {
        // On macOS, application data is stored in ~/Library/Application Support
        return std::string(homeDir) + "/Library/Application Support";
    }
    else if (folderId == CSIDL_PERSONAL) {
        // Documents folder
        return std::string(homeDir) + "/Documents";
    }
    
    return "";
}
#endif

bool FolderManager::createFolderStructure() {
    try {
        std::cout << "Starting folder structure creation..." << std::endl;
        
        // Get system paths
        std::string appSupportPath = getSystemFolder(CSIDL_LOCAL_APPDATA);
        std::string documentsPath = getSystemFolder(CSIDL_PERSONAL);
        
        std::cout << "Got application data path: " << appSupportPath << std::endl;
        std::cout << "Got documents path: " << documentsPath << std::endl;
        
        if (appSupportPath.empty() || documentsPath.empty()) {
            throw std::runtime_error("Failed to get system folders paths");
        }

        // Set up paths
        m_appDataPath = fs::path(appSupportPath) / "iamai";
        m_binPath = m_appDataPath / "bin";
        m_documentsPath = fs::path(documentsPath) / "iamai";
        m_modelsPath = m_documentsPath / "models";

        // Create all directories
        std::cout << "Creating folder structure..." << std::endl;
        
        // Create Application Support folders
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
