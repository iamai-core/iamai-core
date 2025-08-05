#include "../core/interface.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <future>
#include <atomic>

// ImGui
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

struct ChatMessage {
    std::string text;
    bool isUser;
    std::chrono::system_clock::time_point timestamp;

    ChatMessage(const std::string& msg, bool user)
        : text(msg), isUser(user), timestamp(std::chrono::system_clock::now()) {}
};

class ChatDemo {
private:
    std::unique_ptr<Interface> aiInterface;
    std::vector<ChatMessage> messages;
    char inputBuffer[1024] = {0};
    std::atomic<bool> isGenerating{false};
    std::future<std::string> generationFuture;
    std::string currentlyGenerating;

    // UI state
    bool showSettings = false;
    bool showAbout = false;
    bool autoScroll = true;

    // Performance metrics
    std::chrono::high_resolution_clock::time_point lastGenStart;
    double lastGenTime = 0.0;
    int tokensGenerated = 0;

    // Settings
    int maxTokens = 256;
    float temperature = 0.7f;
    bool usePromptFormat = true;

    // Predefined prompts
    std::vector<std::string> quickPrompts = {
        "Tell me about yourself",
        "What can you help me with?",
        "Explain quantum computing in simple terms",
        "Write a short story about AI",
        "What are the benefits of local AI?",
        "How does machine learning work?"
    };

public:
    ChatDemo() {
        // Add welcome message
        messages.emplace_back("Welcome to iamai-core! I'm your personal AI assistant running locally on your device. "
                             "Your conversations are completely private - no data leaves your computer. "
                             "Ask me anything, and let's explore what local AI can do!", false);
    }

    bool Initialize(const std::string& modelPath) {
        try {
            std::cout << "Initializing iamai-core with model: " << modelPath << std::endl;

            Interface::Config config;
            config.max_tokens = maxTokens;
            config.temperature = temperature;
            config.ctx = 4096; // Larger context for better conversations
            config.batch = 32; // Optimized batch size

            aiInterface = std::make_unique<Interface>(modelPath, config);

            if (usePromptFormat) {
                aiInterface->setPromptFormat("Human: {prompt}\n\nAssistant: ");
            }

            std::cout << "iamai-core initialized successfully!" << std::endl;
            return true;

        } catch (const std::exception& e) {
            std::cerr << "Failed to initialize iamai-core: " << e.what() << std::endl;
            messages.emplace_back("❌ Failed to initialize AI model: " + std::string(e.what()), false);
            return false;
        }
    }

    void SendMessage(const std::string& userInput) {
        if (userInput.empty() || isGenerating) return;

        // Add user message
        messages.emplace_back(userInput, true);

        // Start AI generation in background
        isGenerating = true;
        currentlyGenerating = "";
        lastGenStart = std::chrono::high_resolution_clock::now();

        generationFuture = std::async(std::launch::async, [this, userInput]() {
            try {
                return aiInterface->generate(userInput);
            } catch (const std::exception& e) {
                return std::string("❌ Generation error: ") + e.what();
            }
        });
    }

    void Update() {
        // Check if generation is complete
        if (isGenerating && generationFuture.valid()) {
            auto status = generationFuture.wait_for(std::chrono::milliseconds(1));
            if (status == std::future_status::ready) {
                std::string response = generationFuture.get();

                auto endTime = std::chrono::high_resolution_clock::now();
                lastGenTime = std::chrono::duration<double>(endTime - lastGenStart).count();
                tokensGenerated = maxTokens; // Approximate

                messages.emplace_back(response, false);
                isGenerating = false;
            }
        }
    }

    void RenderChat() {
        // Calculate available space for the chat area
        ImVec2 windowSize = ImGui::GetIO().DisplaySize;
        ImVec2 windowPos = ImVec2(0, 0);

        // Set window to fill entire viewport
        ImGui::SetNextWindowPos(windowPos);
        ImGui::SetNextWindowSize(windowSize);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                       ImGuiWindowFlags_NoResize |
                                       ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoBringToFrontOnFocus;

        if (ImGui::Begin("iamai-core Chat Demo", nullptr, window_flags)) {

            // Header with status and controls
            RenderHeader();

            ImGui::Separator();

            // Main chat area - calculate height dynamically
            float headerHeight = ImGui::GetCursorPosY();
            float inputHeight = 80.0f; // Space for input area
            float chatHeight = windowSize.y - headerHeight - inputHeight - 20.0f; // 20px padding

            if (ImGui::BeginChild("ChatHistory", ImVec2(0, chatHeight), ImGuiChildFlags_Border, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
                RenderMessages();
            }
            ImGui::EndChild();

            ImGui::Separator();

            // Input area
            RenderInput();
        }
        ImGui::End();

        // Settings dropdown (no longer as separate window)
        // About dropdown (no longer as separate window)
    }

private:
    void RenderHeader() {
        // Control buttons first (left side)
        if (ImGui::Button("Settings")) {
            showSettings = !showSettings;
        }

        // Store button position for dropdown positioning
        ImVec2 settingsButtonPos = ImGui::GetItemRectMin();
        settingsButtonPos.y += ImGui::GetItemRectSize().y;

        ImGui::SameLine();
        if (ImGui::Button("About")) {
            showAbout = !showAbout;
        }

        // Store button position for dropdown positioning
        ImVec2 aboutButtonPos = ImGui::GetItemRectMin();
        aboutButtonPos.y += ImGui::GetItemRectSize().y;

        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        // Status indicator (right after buttons)
        if (isGenerating) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "● Generating...");
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "● Ready");
        }

        // Performance metrics (same line)
        if (lastGenTime > 0) {
            ImGui::SameLine();
            float tokensPerSec = tokensGenerated / lastGenTime;
            ImGui::Text("| Last: %.2fs (%.1f t/s)", lastGenTime, tokensPerSec);
        }

        // Render dropdown windows
        if (showSettings) {
            ImGui::SetNextWindowPos(settingsButtonPos);
            RenderSettingsDropdown();
        }

        if (showAbout) {
            ImGui::SetNextWindowPos(aboutButtonPos);
            RenderAboutDropdown();
        }
    }

    void RenderMessages() {
        for (size_t i = 0; i < messages.size(); ++i) {
            const auto& msg = messages[i];

            // Message styling based on sender
            if (msg.isUser) {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.3f, 0.8f, 0.3f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
            } else {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
            }

            // Message container
            std::string childId = "msg_" + std::to_string(i);
            if (ImGui::BeginChild(childId.c_str(), ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY)) {

                // Sender label
                if (msg.isUser) {
                    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "You");
                } else {
                    ImGui::TextColored(ImVec4(0.8f, 1.0f, 0.6f, 1.0f), "AI Assistant");
                }

                ImGui::Separator();

                // Message text with word wrapping
                ImGui::PushTextWrapPos(ImGui::GetWindowWidth() - 20);
                ImGui::TextWrapped("%s", msg.text.c_str());
                ImGui::PopTextWrapPos();

                // Timestamp
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

        // Auto-scroll to bottom
        if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
    }

    void RenderInput() {
        // Quick prompts
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

        // Input field
        ImGui::PushItemWidth(-80);
        bool enterPressed = ImGui::InputText("##input", inputBuffer, sizeof(inputBuffer),
                                           ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopItemWidth();

        ImGui::SameLine();
        bool sendClicked = ImGui::Button("Send", ImVec2(70, 0));

        // Send message
        if ((enterPressed || sendClicked) && !isGenerating && strlen(inputBuffer) > 0) {
            SendMessage(std::string(inputBuffer));
            memset(inputBuffer, 0, sizeof(inputBuffer));
        }

        // Focus input field
        if (!isGenerating && !ImGui::IsAnyItemActive()) {
            ImGui::SetKeyboardFocusHere(-1);
        }
    }

    void RenderSettingsDropdown() {
        ImGuiWindowFlags popup_flags = ImGuiWindowFlags_NoMove |
                                      ImGuiWindowFlags_NoResize |
                                      ImGuiWindowFlags_NoTitleBar |
                                      ImGuiWindowFlags_AlwaysAutoResize;

        if (ImGui::Begin("##SettingsDropdown", nullptr, popup_flags)) {
            ImGui::Text("AI Generation Settings");
            ImGui::Separator();

            // Max tokens
            if (ImGui::SliderInt("Max Tokens", &maxTokens, 32, 1024)) {
                if (aiInterface) {
                    aiInterface->setMaxTokens(maxTokens);
                }
            }

            // Temperature
            ImGui::SliderFloat("Temperature", &temperature, 0.1f, 2.0f, "%.1f");

            // Prompt formatting
            if (ImGui::Checkbox("Use Prompt Format", &usePromptFormat)) {
                if (aiInterface) {
                    if (usePromptFormat) {
                        aiInterface->setPromptFormat("Human: {prompt}\n\nAssistant: ");
                    } else {
                        aiInterface->clearPromptFormat();
                    }
                }
            }

            ImGui::Spacing();
            ImGui::Text("UI Settings");
            ImGui::Separator();

            ImGui::Spacing();

            if (ImGui::Button("Clear Chat History", ImVec2(-1, 0))) {
                messages.clear();
                messages.emplace_back("Chat history cleared. How can I help you?", false);
                showSettings = false;
            }

            if (ImGui::Button("Reset to Defaults", ImVec2(-1, 0))) {
                maxTokens = 256;
                temperature = 0.7f;
                usePromptFormat = true;
                showSettings = false;
            }

            if (ImGui::Button("Close", ImVec2(-1, 0))) {
                showSettings = false;
            }

            // Check if we should close (click outside, but not on the settings button)
            if (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
                // Get mouse position and check if it's not over the settings button
                ImVec2 mousePos = ImGui::GetMousePos();
                ImVec2 buttonMin = ImVec2(0, 0); // Settings button position would need to be stored
                ImVec2 buttonMax = ImVec2(80, 25); // Approximate button size

                if (mousePos.x < buttonMin.x || mousePos.x > buttonMax.x ||
                    mousePos.y < buttonMin.y || mousePos.y > buttonMax.y) {
                    showSettings = false;
                }
            }
        }
        ImGui::End();
    }

    void RenderAboutDropdown() {
        ImGuiWindowFlags popup_flags = ImGuiWindowFlags_NoMove |
                                      ImGuiWindowFlags_NoResize |
                                      ImGuiWindowFlags_NoTitleBar |
                                      ImGuiWindowFlags_AlwaysAutoResize;

        if (ImGui::Begin("##AboutDropdown", nullptr, popup_flags)) {
            ImGui::TextWrapped("iamai-core - Personal AI, Simply Yours");
            ImGui::Separator();

            ImGui::PushTextWrapPos(350.0f); // Limit text width for better formatting
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
                // Open website in default browser
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

            // Check if we should close (click outside, but not on the about button)
            if (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
                // Simple approach - just close if clicked outside the window
                showAbout = false;
            }
        }
        ImGui::End();
    }
};

int main(int argc, char* argv[]) {

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create window and renderer
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!SDL_CreateWindowAndRenderer(
        "iamai-core - Personal AI Chat Demo",
        1400, 900,
        SDL_WINDOW_RESIZABLE,
        &window, &renderer)) {
        std::cerr << "SDL_CreateWindowAndRenderer failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderVSync(renderer, 1);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Get DPI scaling for 4K monitor support using SDL3
    float dpiScale = 1.0f;
    SDL_DisplayID displayID = SDL_GetDisplayForWindow(window);
    if (displayID != 0) {
        float contentScale = SDL_GetDisplayContentScale(displayID);
        if (contentScale > 0.0f) {
            dpiScale = contentScale;
            // Clamp scaling to reasonable values
            if (dpiScale < 1.0f) dpiScale = 1.0f;
            if (dpiScale > 3.0f) dpiScale = 3.0f;
        }
    }

    // Apply DPI scaling to fonts and UI
    if (dpiScale > 1.0f) {
        // Scale UI elements
        ImGui::GetStyle().ScaleAllSizes(dpiScale);

        // Load default font with proper scaling
        ImFontConfig fontConfig;
        fontConfig.OversampleH = 2;
        fontConfig.OversampleV = 2;
        fontConfig.SizePixels = 16.0f * dpiScale;
        io.Fonts->AddFontDefault(&fontConfig);

        // Alternative: load custom font if available
        // io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 16.0f * dpiScale, &fontConfig);
    }

    // Setup style - Modern dark theme
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    // Custom color scheme
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.95f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.54f);
    colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Initialize chat demo
    ChatDemo chatDemo;
    bool modelLoaded = chatDemo.Initialize("./models/Llama-3.2-1B-Instruct-Q4_K_M.gguf");

    if (!modelLoaded) {
        std::cerr << "Warning: Model failed to load. Demo will run with limited functionality." << std::endl;
    }

    // Main loop
    bool running = true;
    SDL_Event event;
    ImVec4 clear_color = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);

    std::cout << "Chat demo started successfully! Interact with your local AI assistant." << std::endl;

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                event.window.windowID == SDL_GetWindowID(window)) {
                running = false;
            }
        }

        // Update chat demo
        chatDemo.Update();

        // Start ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Render chat interface
        chatDemo.RenderChat();

        // Render frame
        ImGui::Render();
        SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "Chat demo shutdown completed." << std::endl;
    return 0;
}
