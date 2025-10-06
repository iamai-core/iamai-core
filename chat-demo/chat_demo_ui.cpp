// Prevent Windows API conflicts before any includes
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

#include "chat_demo.h"
#include <iostream>
#include <filesystem>
#include "imgui.h"

void ChatDemo::RenderHeader() {
    if (ImGui::Button("Models")) {
        showModels = !showModels;
        showSettings = false; // Close other dropdowns
        showAbout = false;
    }

    ImVec2 modelsButtonPos = ImGui::GetItemRectMin();
    modelsButtonPos.y += ImGui::GetItemRectSize().y;

    ImGui::SameLine();
    if (ImGui::Button("Settings")) {
        showSettings = !showSettings;
        showModels = false; // Close other dropdowns
        showAbout = false;
    }

    ImVec2 settingsButtonPos = ImGui::GetItemRectMin();
    settingsButtonPos.y += ImGui::GetItemRectSize().y;

    ImGui::SameLine();
    if (ImGui::Button("About")) {
        showAbout = !showAbout;
        showModels = false; // Close other dropdowns
        showSettings = false;
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

    // Show current model if loaded
    if (!currentLoadedModel.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "| Model: %s", currentLoadedModel.c_str());
    }

    if (lastGenTime > 0) {
        ImGui::SameLine();
        float tokensPerSec = tokensGenerated / lastGenTime;
        ImGui::Text("| Last: %.2fs (%.1f t/s)", lastGenTime, tokensPerSec);
    }

    if (showModels) {
        ImGui::SetNextWindowPos(ImVec2(10, modelsButtonPos.y)); // Position with 10px margin
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
    // Set fixed window size with 10px margins
    ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    float margin = 10.0f;
    ImVec2 popupSize = ImVec2(windowSize.x - (margin * 2), windowSize.y - (40 + margin));

    ImGuiWindowFlags popup_flags = ImGuiWindowFlags_NoMove |
                                  ImGuiWindowFlags_NoResize |
                                  ImGuiWindowFlags_NoTitleBar;

    ImGui::SetNextWindowSize(popupSize);

    if (ImGui::Begin("##ModelsDropdown", &showModels, popup_flags)) {
        // Refresh model list when popup opens
        static bool lastShowModels = false;
        if (showModels && !lastShowModels) {
            refreshModelList();
        }
        lastShowModels = showModels;

        ImGui::Text("Model Management");
        ImGui::Separator();

        ImGui::Text("Download Model:");
        ImGui::PushItemWidth(-120); // Leave space for Download button
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

        // Get current model name for highlighting
        std::string currentModelName;
        Interface* currentInterface = modelManager->getCurrentModel();
        if (currentInterface) {
            currentModelName = currentLoadedModel;
        }

        // Create a child window for the model list to handle scrolling
        if (ImGui::BeginChild("ModelList", ImVec2(0, -40), ImGuiChildFlags_Border)) {
            for (const auto& model : availableModels) {
                // Create a unique ID for each model row
                ImGui::PushID(model.c_str());

                // Highlight currently loaded model
                bool isCurrentModel = (model == currentModelName);
                if (isCurrentModel) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.6f, 0.0f, 0.3f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.7f, 0.0f, 0.4f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.8f, 0.0f, 0.5f));
                }

                // Calculate button width to leave space for delete button
                float deleteButtonWidth = 25.0f;
                float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                float availableWidth = ImGui::GetContentRegionAvail().x - deleteButtonWidth - spacing;

                // Model selection button
                if (ImGui::Button(model.c_str(), ImVec2(availableWidth, 0))) {
                    if (modelManager->switchModel(model)) {
                        Interface* interface = modelManager->getCurrentModel();
                        if (interface) {
                            interface->setMaxTokens(maxTokens);
                            if (usePromptFormat) {
                                interface->setPromptFormat("Human: {prompt}\n\nAssistant: ");
                            }
                        }
                        currentLoadedModel = model; // Track current model
                        messages.emplace_back("Switched to model: " + model, false);
                    } else {
                        messages.emplace_back("Failed to switch to model: " + model, false);
                    }
                    showModels = false;
                }

                if (isCurrentModel) {
                    ImGui::PopStyleColor(3);
                }

                // Delete button on the same line
                ImGui::SameLine();
                if (ImGui::Button("X", ImVec2(deleteButtonWidth, 0))) {
                    auto& folder_manager = iamai::FolderManager::getInstance();
                    std::filesystem::path model_path = folder_manager.getModelsPath() / model;
                    try {
                        std::filesystem::remove(model_path);
                        refreshModelList();
                        messages.emplace_back("Deleted model: " + model, false);

                        // Clear current model if it was deleted
                        if (model == currentModelName) {
                            currentLoadedModel.clear();
                        }
                    } catch (const std::exception& e) {
                        messages.emplace_back("Failed to delete model: " + std::string(e.what()), false);
                    }
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Delete model");
                }

                ImGui::PopID();
            }
        }
        ImGui::EndChild();

        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(-1, 0))) {
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

                ImGui::SameLine(ImGui::GetWindowWidth() - 50);
                std::string copyButtonId = "📋##" + std::to_string(i);
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

            // auto time_t = std::chrono::system_clock::to_time_t(msg.timestamp);
            // ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            // ImGui::SetWindowFontScale(0.75f);
            // ImGui::Text("%.24s", std::ctime(&time_t));
            // ImGui::SetWindowFontScale(1.0f);
            // ImGui::PopStyleColor();
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

    ImGui::SetWindowFontScale(0.75f);
    for (size_t i = 0; i < quickPrompts.size(); ++i) {
        if (i > 0) ImGui::SameLine();

        std::string buttonId = "##quick" + std::to_string(i);
        if (ImGui::SmallButton((quickPrompts[i].substr(0, 18) + "...").c_str())) {
            strcpy_s(inputBuffer, sizeof(inputBuffer), quickPrompts[i].c_str());
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", quickPrompts[i].c_str());
        }
    }
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();

    if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyItemFocused()) {
        ImGui::SetKeyboardFocusHere(0);
    }

    ImGui::PushItemWidth(-80);
    bool enterPressed = ImGui::InputText("##input", inputBuffer, sizeof(inputBuffer),
                                       ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    bool sendClicked = ImGui::Button("Send", ImVec2(70, 0));

    if ((enterPressed || sendClicked) && !isGenerating && strlen(inputBuffer) > 0) {
        SendChatMessage(std::string(inputBuffer));
        memset(inputBuffer, 0, sizeof(inputBuffer));
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

        ImGui::TextWrapped("iamai-core is an open-source personal AI that runs entirely on your device. "
                         "No cloud services, no data sharing - just download and start.");

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
