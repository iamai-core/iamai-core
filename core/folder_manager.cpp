#include "folder_manager.h"
#include <stdexcept>
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
    #include <knownfolders.h>
    #pragma comment(lib, "shell32.lib")
    #pragma comment(lib, "ole32.lib")
#endif

namespace iamai {

namespace fs = std::filesystem;

FolderManager& FolderManager::getInstance() {
    static FolderManager instance;
    return instance;
}

std::filesystem::path FolderManager::getSystemAppDataPath() const {
#ifdef _WIN32
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path))) {
        std::filesystem::path result(path);
        CoTaskMemFree(path);
        return result;
    }
    throw std::runtime_error("Failed to get Windows AppData path");
#elif defined(__APPLE__)
    const char* home = std::getenv("HOME");
    if (!home) {
        throw std::runtime_error("Failed to get HOME directory");
    }
    return fs::path(home) / "Library" / "Application Support";
#else // Linux and other Unix-like systems
    const char* xdgConfigHome = std::getenv("XDG_CONFIG_HOME");
    if (xdgConfigHome) {
        return fs::path(xdgConfigHome);
    }
    const char* home = std::getenv("HOME");
    if (!home) {
        throw std::runtime_error("Failed to get HOME directory");
    }
    return fs::path(home) / ".config";
#endif
}

std::filesystem::path FolderManager::getSystemDocumentsPath() const {
#ifdef _WIN32
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path))) {
        std::filesystem::path result(path);
        CoTaskMemFree(path);
        return result;
    }
    throw std::runtime_error("Failed to get Windows Documents path");
#elif defined(__APPLE__)
    const char* home = std::getenv("HOME");
    if (!home) {
        throw std::runtime_error("Failed to get HOME directory");
    }
    return fs::path(home) / "Documents";
#else // Linux and other Unix-like systems
    const char* xdgDocuments = std::getenv("XDG_DOCUMENTS_DIR");
    if (xdgDocuments) {
        return fs::path(xdgDocuments);
    }
    const char* home = std::getenv("HOME");
    if (!home) {
        throw std::runtime_error("Failed to get HOME directory");
    }
    return fs::path(home) / "Documents";
#endif
}

bool FolderManager::createFolderStructure() {
    try {
        std::cout << "Starting folder structure creation..." << std::endl;

        // Get system paths
        std::filesystem::path appDataPath = getSystemAppDataPath();
        std::filesystem::path documentsPath = getSystemDocumentsPath();

        std::cout << "Got AppData path: " << appDataPath << std::endl;
        std::cout << "Got Documents path: " << documentsPath << std::endl;

        // Set up paths
        m_appDataPath = appDataPath / "iamai";
        m_binPath = m_appDataPath / "bin";
        m_documentsPath = documentsPath / "iamai";
        m_modelsPath = m_documentsPath / "models";

        // Create all directories
        std::cout << "Creating folder structure..." << std::endl;

        fs::create_directories(m_appDataPath);
        std::cout << "Created: " << m_appDataPath << std::endl;

        fs::create_directories(m_binPath);
        std::cout << "Created: " << m_binPath << std::endl;

        fs::create_directories(m_documentsPath);
        std::cout << "Created: " << m_documentsPath << std::endl;

        fs::create_directories(m_modelsPath);
        std::cout << "Created: " << m_modelsPath << std::endl;

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
