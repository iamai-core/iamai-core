#include "chat_demo.h"
#include <iostream>
#include <filesystem>
#include "imgui.h"

void ChatDemo::RenderHeader() {
    if (ImGui::Button("Models")) {
        showModels = !showModels;
    }

    ImVec2 modelsButtonPos = ImGui::GetItemRectMin();
    modelsButtonPos.y += ImGui::GetItemRectSize().y;

    ImGui::SameLine();
    if (ImGui::Button("Settings")) {
        showSettings = !showSettings;
    }

    ImVec2 settingsButtonPos = ImGui::GetItemRectMin();
    settingsButtonPos.y += ImGui::GetItemRectSize().y;

    ImGui::SameLine();
    if (ImGui::Button("About")) {
        showAbout = !showAbout;
    }

    ImVec2 aboutButtonPos = ImGui::GetItemRectMin();
    aboutButtonPos.y += ImGui::GetItemRectSize().y;

    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();

    if (isGenerating) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "● Generating...");
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "● Ready");
    }

    if (lastGenTime > 0) {
        ImGui::SameLine();
        float tokensPerSec = tokensGenerated / lastGenTime;
        ImGui::Text("| Last: %.2fs (%.1f t/s)", lastGenTime, tokensPerSec);
    }

    if (showModels) {
        ImGui::SetNextWindowPos(modelsButtonPos);
        RenderModelsDropdown();
    }

    if (showSettings) {
        ImGui::SetNextWindowPos(settingsButtonPos);
        RenderSettingsDropdown();
    }

    if (showAbout) {
        ImGui::SetNextWindowPos(aboutButtonPos);
        RenderAboutDropdown();
    }
}

void ChatDemo::RenderModelsDropdown() {
    ImGuiWindowFlags popup_flags = ImGuiWindowFlags_NoMove |
                                  ImGuiWindowFlags_NoResize |
                                  ImGuiWindowFlags_NoTitleBar |
                                  ImGuiWindowFlags_AlwaysAutoResize;

    if (ImGui::Begin("##ModelsDropdown", nullptr, popup_flags)) {
        ImGui::Text("Model Management");
        ImGui::Separator();

        ImGui::Text("Download Model:");
        ImGui::PushItemWidth(400);
        ImGui::InputText("##downloadUrl", downloadUrlBuffer, sizeof(downloadUrlBuffer));
        ImGui::PopItemWidth();

        ImGui::SameLine();
        if (ImGui::Button("Download") && !downloadProgress.active) {
            std::string url = downloadUrlBuffer;
            std::string filename = std::filesystem::path(url.substr(url.find_last_of('/') + 1)).filename().string();

            size_t questionPos = filename.find('?');
            if (questionPos != std::string::npos) {
                filename = filename.substr(0, questionPos);
            }

            downloadProgress.filename = filename;
            downloadProgress.downloaded = 0.0;
            downloadProgress.total = 0.0;
            downloadProgress.active = true;
            downloadProgress.complete = false;
            downloadProgress.error = false;
            downloadProgress.error_message.clear();

            downloadFuture = std::async(std::launch::async, [this, url, filename]() {
                return downloadModel(url, filename);
            });
        }

        if (downloadProgress.active) {
            ImGui::Text("Downloading: %s", downloadProgress.filename.c_str());
            if (downloadProgress.total > 0) {
                float progress = static_cast<float>(downloadProgress.downloaded / downloadProgress.total);
                ImGui::ProgressBar(progress, ImVec2(-1, 0),
                    (std::to_string(static_cast<int>(downloadProgress.downloaded / 1024 / 1024)) +
                     " / " + std::to_string(static_cast<int>(downloadProgress.total / 1024 / 1024)) + " MB").c_str());
            } else {
                ImGui::ProgressBar(0.0f, ImVec2(-1, 0), "Connecting...");
            }
        }

        if (downloadProgress.error) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: %s", downloadProgress.error_message.c_str());
        }

        if (downloadProgress.complete) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Download complete!");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Available Models:");

        for (const auto& model : availableModels) {
            if (ImGui::Selectable(model.c_str())) {
                if (modelManager->switchModel(model)) {
                    Interface* interface = modelManager->getCurrentModel();
                    if (interface) {
                        interface->setMaxTokens(maxTokens);
                        if (usePromptFormat) {
                            interface->setPromptFormat("Human: {prompt}\n\nAssistant: ");
                        }
                    }
                    messages.emplace_back("Switched to model: " + model, false);
                } else {
                    messages.emplace_back("Failed to switch to model: " + model, false);
                }
                showModels = false;
            }

            ImGui::SameLine(ImGui::GetWindowWidth() - 30);
            std::string deleteButtonId = "X##" + model;
            if (ImGui::SmallButton(deleteButtonId.c_str())) {
                auto& folder_manager = iamai::FolderManager::getInstance();
                std::filesystem::path model_path = folder_manager.getModelsPath() / model;
                try {
                    std::filesystem::remove(model_path);
                    refreshModelList();
                    messages.emplace_back("Deleted model: " + model, false);
                } catch (const std::exception& e) {
                    messages.emplace_back("Failed to delete model: " + std::string(e.what()), false);
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Delete model");
            }
        }

        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(-1, 0))) {
            showModels = false;
        }

        if (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
            showModels = false;
        }
    }
    ImGui::End();
}

void ChatDemo::RenderMessages() {
    for (size_t i = 0; i < messages.size(); ++i) {
        const auto& msg = messages[i];

        if (msg.isUser) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.3f, 0.8f, 0.3f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
        } else {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
        }

        std::string childId = "msg_" + std::to_string(i);
        if (ImGui::BeginChild(childId.c_str(), ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY)) {

            if (msg.isUser) {
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "You");
            } else {
                ImGui::TextColored(ImVec4(0.8f, 1.0f, 0.6f, 1.0f), "AI Companion");

                ImGui::SameLine(ImGui::GetWindowWidth() - 60);
                std::string copyButtonId = "Copy##" + std::to_string(i);
                if (ImGui::SmallButton(copyButtonId.c_str())) {
                    SDL_SetClipboardText(msg.text.c_str());
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Copy message to clipboard");
                }
            }

            ImGui::Separator();

            ImGui::PushTextWrapPos(ImGui::GetWindowWidth() - 20);
            ImGui::TextWrapped("%s", msg.text.c_str());
            ImGui::PopTextWrapPos();

            auto time_t = std::chrono::system_clock::to_time_t(msg.timestamp);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::Text("%.24s", std::ctime(&time_t));
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        ImGui::Spacing();
    }

    if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
}

void ChatDemo::RenderInput() {
    ImGui::Text("Quick prompts:");
    for (size_t i = 0; i < quickPrompts.size(); ++i) {
        if (i > 0) ImGui::SameLine();

        std::string buttonId = "##quick" + std::to_string(i);
        if (ImGui::SmallButton((quickPrompts[i].substr(0, 15) + "...").c_str())) {
            if (!isGenerating) {
                strcpy_s(inputBuffer, sizeof(inputBuffer), quickPrompts[i].c_str());
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", quickPrompts[i].c_str());
        }
    }

    ImGui::Spacing();

    ImGui::PushItemWidth(-80);
    bool enterPressed = ImGui::InputText("##input", inputBuffer, sizeof(inputBuffer),
                                       ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    bool sendClicked = ImGui::Button("Send", ImVec2(70, 0));

    if ((enterPressed || sendClicked) && !isGenerating && strlen(inputBuffer) > 0) {
        SendMessage(std::string(inputBuffer));
        memset(inputBuffer, 0, sizeof(inputBuffer));
    }

    if (!isGenerating && !ImGui::IsAnyItemActive()) {
        ImGui::SetKeyboardFocusHere(-1);
    }
}

void ChatDemo::RenderSettingsDropdown() {
    ImGuiWindowFlags popup_flags = ImGuiWindowFlags_NoMove |
                                  ImGuiWindowFlags_NoResize |
                                  ImGuiWindowFlags_NoTitleBar |
                                  ImGuiWindowFlags_AlwaysAutoResize;

    if (ImGui::Begin("##SettingsDropdown", nullptr, popup_flags)) {
        ImGui::Text("AI Generation Settings");
        ImGui::Separator();

        if (ImGui::SliderInt("Max Tokens", &maxTokens, 32, 1024)) {
            Interface* interface = modelManager->getCurrentModel();
            if (interface) {
                interface->setMaxTokens(maxTokens);
            }
            saveSettings();
        }

        if (ImGui::SliderFloat("Temperature", &temperature, 0.1f, 2.0f, "%.1f")) {
            saveSettings();
        }

        if (ImGui::Checkbox("Use Prompt Format", &usePromptFormat)) {
            Interface* interface = modelManager->getCurrentModel();
            if (interface) {
                if (usePromptFormat) {
                    interface->setPromptFormat("Human: {prompt}\n\nAssistant: ");
                } else {
                    interface->clearPromptFormat();
                }
            }
            saveSettings();
        }

        ImGui::Spacing();

        if (ImGui::Button("Clear Chat History", ImVec2(-1, 0))) {
            messages.clear();
            messages.emplace_back("Chat history cleared. How can I help you?", false);
            showSettings = false;
        }

        if (ImGui::Button("Close", ImVec2(-1, 0))) {
            showSettings = false;
        }

        if (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
            showSettings = false;
        }
    }
    ImGui::End();
}

void ChatDemo::RenderAboutDropdown() {
    ImGuiWindowFlags popup_flags = ImGuiWindowFlags_NoMove |
                                  ImGuiWindowFlags_NoResize |
                                  ImGuiWindowFlags_NoTitleBar |
                                  ImGuiWindowFlags_AlwaysAutoResize;

    if (ImGui::Begin("##AboutDropdown", nullptr, popup_flags)) {
        ImGui::TextWrapped("iamai-core - Personal AI, Simply Yours");
        ImGui::Separator();

        ImGui::PushTextWrapPos(350.0f);
        ImGui::TextWrapped("iamai-core is an open-source personal AI that runs entirely on your device. "
                         "No cloud services, no data sharing - just download and start.");
        ImGui::PopTextWrapPos();

        ImGui::Spacing();
        ImGui::Text("Key Features:");
        ImGui::BulletText("100%% Local - Your data never leaves your device");
        ImGui::BulletText("Privacy First - Complete control over your conversations");
        ImGui::BulletText("Real-time Learning - Adapts to your preferences");
        ImGui::BulletText("Cross-platform - Windows, macOS, Linux support");
        ImGui::BulletText("Developer Friendly - Easy integration with plugins");

        ImGui::Spacing();
        ImGui::Text("Technology Stack:");
        ImGui::BulletText("Core: C/C++ for performance");
        ImGui::BulletText("AI Engine: llama.cpp integration");
        ImGui::BulletText("UI: Dear ImGui + SDL3");
        ImGui::BulletText("Models: GGUF format support");

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::Button("Visit Website", ImVec2(-1, 0))) {
            #ifdef _WIN32
            system("start https://iamai-core.org");
            #elif defined(__APPLE__)
            system("open https://iamai-core.org");
            #else
            system("xdg-open https://iamai-core.org");
            #endif
            showAbout = false;
        }

        if (ImGui::Button("GitHub Repository", ImVec2(-1, 0))) {
            #ifdef _WIN32
            system("start https://github.com/iamai-core");
            #elif defined(__APPLE__)
            system("open https://github.com/iamai-core");
            #else
            system("xdg-open https://github.com/iamai-core");
            #endif
            showAbout = false;
        }

        if (ImGui::Button("Close", ImVec2(-1, 0))) {
            showAbout = false;
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Version 1.0.0 - MIT License");

        if (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
            showAbout = false;
        }
    }
    ImGui::End();
}
