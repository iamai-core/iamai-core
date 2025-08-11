#pragma once

// Prevent Windows.h from defining macros that conflict with our code
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#ifndef NOGDI
#define NOGDI
#endif
#ifndef NOUSER
#define NOUSER
#endif
#endif

// Include curl headers before other headers to avoid conflicts
#include <curl/curl.h>

#include "../core/interface.h"
#include "../core/folder_manager.h"
#include "../core/model_manager.h"
#include "settings_manager.h"
#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include <chrono>
#include <future>
#include <atomic>
#include <memory>

struct DownloadProgress {
    std::atomic<double> downloaded{0.0};
    std::atomic<double> total{0.0};
    std::atomic<bool> active{false};
    std::atomic<bool> complete{false};
    std::atomic<bool> error{false};
    std::string filename;
    std::string error_message;
};

struct ChatMessage {
    std::string text;
    bool isUser;
    std::chrono::system_clock::time_point timestamp;

    ChatMessage(const std::string& msg, bool user);
};

class ChatDemo {
private:
    std::unique_ptr<iamai::ModelManager> modelManager;
    std::unique_ptr<SettingsManager> settingsManager;
    std::vector<ChatMessage> messages;
    char inputBuffer[1024] = {0};
    std::atomic<bool> isGenerating{false};
    std::future<std::string> generationFuture;
    std::string currentlyGenerating;

    // UI state
    bool showSettings = false;
    bool showAbout = false;
    bool showModels = false;
    bool autoScroll = true;

    // Model management
    std::vector<std::string> availableModels;
    std::string currentLoadedModel;
    char downloadUrlBuffer[1024] = "https://huggingface.co/unsloth/Llama-3.2-1B-Instruct-GGUF/resolve/main/Llama-3.2-1B-Instruct-Q4_K_M.gguf?download=true";
    DownloadProgress downloadProgress;
    std::future<bool> downloadFuture;

    // Performance metrics
    std::chrono::high_resolution_clock::time_point lastGenStart;
    double lastGenTime = 0.0;
    int tokensGenerated = 0;

    // Settings
    int maxTokens = 256;
    float temperature = 0.7f;
    bool usePromptFormat = true;

    std::vector<std::string> quickPrompts = {
        "Tell me about yourself",
        "What can you help me with?",
        "Explain quantum computing in simple terms",
        "Write a short story about AI",
        "What are the benefits of local AI?",
        "How does machine learning work?"
    };

    // Helper methods
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static int ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
    bool downloadModel(const std::string& url, const std::string& filename);
    void refreshModelList();
    void loadSettings();
    void saveSettings();

    // Render methods
    void RenderHeader();
    void RenderModelsDropdown();
    void RenderMessages();
    void RenderInput();
    void RenderSettingsDropdown();
    void RenderAboutDropdown();

public:
    ChatDemo();
    bool Initialize(const std::string& modelPath = "");
    void SendChatMessage(const std::string& userInput);
    void Update();
    void RenderChatInitial(SDL_Window* window);
    void RenderChat();
};
