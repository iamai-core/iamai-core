#include "chat_demo.h"
#include <iostream>
#include <filesystem>
#include <curl/curl.h>
#include "imgui.h"

ChatMessage::ChatMessage(const std::string& msg, bool user)
    : text(msg), isUser(user), timestamp(std::chrono::system_clock::now()) {}

ChatDemo::ChatDemo() {
    settingsManager = std::make_unique<SettingsManager>("ChatDemo");

    auto& folder_manager = iamai::FolderManager::getInstance();
    folder_manager.createFolderStructure();

    modelManager = std::make_unique<iamai::ModelManager>();
    refreshModelList();

    messages.emplace_back("Welcome to iamai-core! I'm your personal AI companion running locally on your device. "
                         "Your conversations are completely private - no data leaves your computer. "
                         "Ask me anything, and let's explore what local AI can do!", false);
}

size_t ChatDemo::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    FILE* fp = static_cast<FILE*>(userp);
    return fwrite(contents, size, nmemb, fp);
}

int ChatDemo::ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    DownloadProgress* progress = static_cast<DownloadProgress*>(clientp);
    progress->total = static_cast<double>(dltotal);
    progress->downloaded = static_cast<double>(dlnow);
    return 0;
}

bool ChatDemo::downloadModel(const std::string& url, const std::string& filename) {
    auto& folder_manager = iamai::FolderManager::getInstance();
    std::filesystem::path model_path = folder_manager.getModelsPath() / filename;

    CURL* curl = curl_easy_init();
    if (!curl) {
        downloadProgress.error = true;
        downloadProgress.error_message = "Failed to initialize curl";
        return false;
    }

    FILE* fp = fopen(model_path.string().c_str(), "wb");
    if (!fp) {
        curl_easy_cleanup(curl);
        downloadProgress.error = true;
        downloadProgress.error_message = "Failed to create file";
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &downloadProgress);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3600L);

    CURLcode res = curl_easy_perform(curl);
    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::filesystem::remove(model_path);
        downloadProgress.error = true;
        downloadProgress.error_message = curl_easy_strerror(res);
        return false;
    }

    downloadProgress.complete = true;
    refreshModelList();
    return true;
}

void ChatDemo::refreshModelList() {
    availableModels = modelManager->listModels();
}

void ChatDemo::loadSettings() {
    maxTokens = settingsManager->getInt("maxTokens", maxTokens);
    temperature = settingsManager->getFloat("temperature", temperature);
    usePromptFormat = settingsManager->getBool("usePromptFormat", usePromptFormat);
}

void ChatDemo::saveSettings() {
    settingsManager->setInt("maxTokens", maxTokens);
    settingsManager->setFloat("temperature", temperature);
    settingsManager->setBool("usePromptFormat", usePromptFormat);
}

bool ChatDemo::Initialize(const std::string& modelPath) {
    try {
        if (!modelPath.empty()) {
            std::cout << "Initializing iamai-core with model: " << modelPath << std::endl;

            if (modelManager->switchModel(modelPath)) {
                Interface* interface = modelManager->getCurrentModel();
                if (interface) {
                    interface->setMaxTokens(maxTokens);

                    if (usePromptFormat) {
                        interface->setPromptFormat("Human: {prompt}\n\nAssistant: ");
                    }

                    std::cout << "iamai-core initialized successfully!" << std::endl;
                    return true;
                }
            }
        }

        std::cout << "No model specified or failed to load. Use Models button to select one." << std::endl;
        return false;

    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize iamai-core: " << e.what() << std::endl;
        messages.emplace_back("❌ Failed to initialize AI model: " + std::string(e.what()), false);
        return false;
    }
}

void ChatDemo::SendMessage(const std::string& userInput) {
    if (userInput.empty() || isGenerating) return;

    Interface* interface = modelManager->getCurrentModel();
    if (!interface) {
        messages.emplace_back("❌ No model loaded. Please select a model first.", false);
        return;
    }

    messages.emplace_back(userInput, true);

    isGenerating = true;
    currentlyGenerating = "";
    lastGenStart = std::chrono::high_resolution_clock::now();

    generationFuture = std::async(std::launch::async, [this, userInput, interface]() {
        try {
            return interface->generate(userInput);
        } catch (const std::exception& e) {
            return std::string("❌ Generation error: ") + e.what();
        }
    });
}

void ChatDemo::Update() {
    if (isGenerating && generationFuture.valid()) {
        auto status = generationFuture.wait_for(std::chrono::milliseconds(1));
        if (status == std::future_status::ready) {
            std::string response = generationFuture.get();

            auto endTime = std::chrono::high_resolution_clock::now();
            lastGenTime = std::chrono::duration<double>(endTime - lastGenStart).count();
            tokensGenerated = maxTokens;

            messages.emplace_back(response, false);
            isGenerating = false;
        }
    }

    if (downloadFuture.valid()) {
        auto status = downloadFuture.wait_for(std::chrono::milliseconds(1));
        if (status == std::future_status::ready) {
            downloadFuture.get();
            downloadProgress.active = false;
        }
    }
}

void ChatDemo::RenderChatInitial(SDL_Window* window) {
    loadSettings();

    if (ImGui::Begin("iamai-core Chat Demo", nullptr, 0)) {
        ImVec2 windowSize = ImGui::GetWindowSize();
        SDL_SetWindowSize(window, (int)windowSize.x, (int)windowSize.y);
    }
    ImGui::End();
}

void ChatDemo::RenderChat() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                   ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("iamai-core Chat Demo", nullptr, window_flags)) {
        ImVec2 windowSize = ImGui::GetIO().DisplaySize;
        ImGui::SetWindowPos(ImVec2(0, 0));
        ImGui::SetWindowSize(windowSize);

        RenderHeader();
        ImGui::Separator();

        float headerHeight = ImGui::GetCursorPosY();
        float inputHeight = 80.0f;
        float chatHeight = windowSize.y - headerHeight - inputHeight - 20.0f;

        if (ImGui::BeginChild("ChatHistory", ImVec2(0, chatHeight), ImGuiChildFlags_Border, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            RenderMessages();
        }
        ImGui::EndChild();

        ImGui::Separator();
        RenderInput();
    }
    ImGui::End();
}
