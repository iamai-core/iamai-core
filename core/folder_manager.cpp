// Updated folder_manager.cpp
#include "folder_manager.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
    #include <knownfolders.h>
    #pragma comment(lib, "shell32.lib")
    #pragma comment(lib, "ole32.lib")
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <unistd.h>
#else
    #include <unistd.h>
    #include <linux/limits.h>
#endif

namespace iamai {

namespace fs = std::filesystem;

FolderManager& FolderManager::getInstance() {
    static FolderManager instance;
    return instance;
}

std::filesystem::path FolderManager::getSystemConfigPath() const {
#ifdef _WIN32
    // Windows: %APPDATA% (Roaming profile data)
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path))) {
        std::filesystem::path result(path);
        CoTaskMemFree(path);
        return result;
    }
    throw std::runtime_error("Failed to get Windows AppData path");
#elif defined(__APPLE__)
    // macOS: ~/Library/Application Support
    const char* home = std::getenv("HOME");
    if (!home) {
        throw std::runtime_error("Failed to get HOME directory");
    }
    return fs::path(home) / "Library" / "Application Support";
#else
    // Linux: ~/.config (XDG Base Directory)
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

std::filesystem::path FolderManager::getSystemDataPath() const {
#ifdef _WIN32
    // Windows: %LOCALAPPDATA% (Local machine data)
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path))) {
        std::filesystem::path result(path);
        CoTaskMemFree(path);
        return result;
    }
    throw std::runtime_error("Failed to get Windows LocalAppData path");
#elif defined(__APPLE__)
    // macOS: ~/Library/Application Support (same as config on macOS)
    const char* home = std::getenv("HOME");
    if (!home) {
        throw std::runtime_error("Failed to get HOME directory");
    }
    return fs::path(home) / "Library" / "Application Support";
#else
    // Linux: ~/.local/share (XDG Base Directory)
    const char* xdgDataHome = std::getenv("XDG_DATA_HOME");
    if (xdgDataHome) {
        return fs::path(xdgDataHome);
    }
    const char* home = std::getenv("HOME");
    if (!home) {
        throw std::runtime_error("Failed to get HOME directory");
    }
    return fs::path(home) / ".local" / "share";
#endif
}

std::filesystem::path FolderManager::getSystemCachePath() const {
#ifdef _WIN32
    // Windows: %TEMP% for temporary/cache files
    const char* temp = std::getenv("TEMP");
    if (temp) {
        return fs::path(temp);
    }
    // Fallback to %TMP%
    temp = std::getenv("TMP");
    if (temp) {
        return fs::path(temp);
    }
    throw std::runtime_error("Failed to get Windows temp path");
#elif defined(__APPLE__)
    // macOS: ~/Library/Caches
    const char* home = std::getenv("HOME");
    if (!home) {
        throw std::runtime_error("Failed to get HOME directory");
    }
    return fs::path(home) / "Library" / "Caches";
#else
    // Linux: ~/.cache (XDG Base Directory)
    const char* xdgCacheHome = std::getenv("XDG_CACHE_HOME");
    if (xdgCacheHome) {
        return fs::path(xdgCacheHome);
    }
    const char* home = std::getenv("HOME");
    if (!home) {
        throw std::runtime_error("Failed to get HOME directory");
    }
    return fs::path(home) / ".cache";
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
#else
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

std::filesystem::path FolderManager::getCurrentExecutablePath() const {
#ifdef _WIN32
    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0) {
        throw std::runtime_error("Failed to get executable path on Windows");
    }
    return fs::path(path).parent_path();
#elif defined(__APPLE__)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        throw std::runtime_error("Failed to get executable path on macOS");
    }
    return fs::path(path).parent_path();
#else
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) {
        throw std::runtime_error("Failed to get executable path on Linux");
    }
    path[len] = '\0';
    return fs::path(path).parent_path();
#endif
}

bool FolderManager::createFolderStructure() {
    try {
        std::cout << "Creating iamai-core folder structure..." << std::endl;

        // Get system paths
        std::filesystem::path configBase = getSystemConfigPath();
        std::filesystem::path dataBase = getSystemDataPath();
        std::filesystem::path cacheBase = getSystemCachePath();
        std::filesystem::path documentsBase = getSystemDocumentsPath();
        std::filesystem::path executableBase = getCurrentExecutablePath();

        // Set up iamai-core specific paths

        // PERSISTENT DATA (survives app updates)
        m_configPath = configBase / "iamai-core";              // Settings, INI files
        m_dataPath = dataBase / "iamai-core";                  // Database, logs, persistent data
        m_cachePath = cacheBase / "iamai-core";                // Temporary files, cache

        // Models go directly in Documents/iamai-core for easy user access
        // This is the main user-facing directory for the application
        m_modelsPath = documentsBase / "iamai-core";

        // APPLICATION BINARIES (may be replaced on updates)
        m_appBinaryPath = executableBase;                      // Where the executable lives
        m_runtimeLibsPath = executableBase;                    // CUDA DLLs alongside executable

        fs::create_directories(m_configPath);
        std::cout << "Created config: " << m_configPath << std::endl;

        fs::create_directories(m_dataPath);
        std::cout << "Created data: " << m_dataPath << std::endl;

        fs::create_directories(m_cachePath);
        std::cout << "Created cache: " << m_cachePath << std::endl;

        fs::create_directories(m_modelsPath);
        std::cout << "Created models: " << m_modelsPath << std::endl;

        // Application binary directories (may already exist)
        std::cout << "Application binaries: " << m_appBinaryPath << std::endl;
        std::cout << "Runtime libraries: " << m_runtimeLibsPath << std::endl;

        // Create a README in the models directory to help users
        std::filesystem::path readmePath = m_modelsPath / "README.txt";
        if (!fs::exists(readmePath)) {
            std::ofstream readme(readmePath);
            readme << "iamai-core Models Directory\n"
                   << "===========================\n\n"
                   << "This folder contains your AI models in GGUF format.\n"
                   << "You can:\n"
                   << "- Download models from Hugging Face\n"
                   << "- Copy .gguf files directly to this folder\n"
                   << "- Use the app's download feature\n\n"
                   << "Note: Larger models require more RAM and are slower.\n"
                   << "Start with smaller models (1B-3B parameters) for testing.\n";
        }

        std::cout << "Folder structure created successfully!" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error creating folder structure: " << e.what() << std::endl;
        return false;
    }
}

// Getter implementations
fs::path FolderManager::getConfigPath() const {
    return m_configPath;
}

fs::path FolderManager::getDataPath() const {
    return m_dataPath;
}

fs::path FolderManager::getModelsPath() const {
    return m_modelsPath;
}

fs::path FolderManager::getCachePath() const {
    return m_cachePath;
}

fs::path FolderManager::getAppBinaryPath() const {
    return m_appBinaryPath;
}

fs::path FolderManager::getRuntimeLibsPath() const {
    return m_runtimeLibsPath;
}

} // namespace iamai
