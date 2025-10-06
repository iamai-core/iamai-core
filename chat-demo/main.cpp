#include "chat_demo.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <curl/curl.h>
#include "../core/folder_manager.h"

// ImGui
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

int main(int argc, char* argv[]) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_DisplayID primaryDisplay = SDL_GetPrimaryDisplay();
    SDL_Rect usableBounds;
    int windowWidth = 800; int windowHeight = 600;
    if (SDL_GetDisplayUsableBounds(primaryDisplay, &usableBounds)) {
        windowWidth = static_cast<int>(usableBounds.w * 0.5f);
        windowHeight = static_cast<int>(usableBounds.h * 0.5f);
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!SDL_CreateWindowAndRenderer(
        "iamai-core - Personal AI Chat Demo",
        windowWidth, windowHeight,
        SDL_WINDOW_RESIZABLE,
        &window, &renderer)) {
        std::cerr << "SDL_CreateWindowAndRenderer failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderVSync(renderer, 1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    auto& folder_manager = iamai::FolderManager::getInstance();

    // Set ImGui ini file path to config directory
    try {
        folder_manager.createFolderStructure(); // Ensure directories exist
        static const std::string ini_path = (folder_manager.getConfigPath() / "chat-demo-gui.ini").string();
        io.IniFilename = ini_path.c_str();
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to set config path, using default ini: " << e.what() << std::endl;
        io.IniFilename = "chat-demo-gui.ini"; // Fallback to current directory
    }

    // Get screen scale to scale UI elements
    float dpiScale = 1.0f;
    SDL_DisplayID displayID = SDL_GetDisplayForWindow(window);
    if (displayID != 0) {
        float contentScale = SDL_GetDisplayContentScale(displayID);
        if (contentScale > 0.0f) {
            dpiScale = contentScale;
            if (dpiScale < 1.0f) dpiScale = 1.0f;
            if (dpiScale > 3.0f) dpiScale = 3.0f;
        }
    }

    // Scale all UI elements
    if (dpiScale > 1.0f) {
        ImGui::GetStyle().ScaleAllSizes(dpiScale);
    }

    // Try to load fonts
    ImFontConfig fontConfig;
    fontConfig.OversampleH = 2;
    fontConfig.OversampleV = 2;

    std::filesystem::path libsPath = folder_manager.getRuntimeLibsPath();
    const char* fonts[] = {
        "NotoSansSC-Regular.ttf",
        "NotoSansJP-Regular.ttf",
        "NotoSans-Regular.ttf",
        "NotoSerifSC-Regular.ttf",
        "NotoSansSC-SemiBold.ttf",
        nullptr
    };

    bool fontLoaded = false;
    for (int i = 0; fonts[i] != nullptr; ++i) {
        std::filesystem::path fullPath = libsPath / fonts[i];

        if (std::filesystem::exists(fullPath)) {
            std::string pathStr = fullPath.string();
            ImFont* font = io.Fonts->AddFontFromFileTTF(
                pathStr.c_str(),
                22.0f * dpiScale,
                &fontConfig,
                nullptr
            );
            if (font) {
                fontLoaded = true;
                std::cout << "Loaded font: " << fullPath << std::endl;
                break;
            }
        }
    }

    if (!fontLoaded) {
        std::cerr << "Warning: Could not load Unicode font, using default" << std::endl;
        fontConfig.SizePixels = 16.0f * dpiScale;
        io.Fonts->AddFontDefault(&fontConfig);
    }

    ImFontConfig emojiConfig;
    emojiConfig.OversampleH = 2;
    emojiConfig.OversampleV = 2;
    emojiConfig.MergeMode = true;  // Merge into previous font

    std::filesystem::path emoji_path = (folder_manager.getRuntimeLibsPath() / "NotoEmoji-Regular.ttf");
    std::string emoji_path_str = emoji_path.string();

    if (std::filesystem::exists(emoji_path)) {
        ImFont* emoji_font = io.Fonts->AddFontFromFileTTF(
            emoji_path_str.c_str(),
            18.0f * dpiScale,
            &emojiConfig,
            nullptr
        );
        if (emoji_font) {
            std::cout << "Loaded emoji: " << emoji_path << std::endl;
        }
    }


    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.95f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.54f);
    colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    ImVec4 clear_color = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ChatDemo chatDemo;
    chatDemo.RenderChatInitial(window);

    ImGui::Render();
    SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);

    std::cout << "Chat demo started successfully! Select a model to begin." << std::endl;

    bool running = true;
    SDL_Event event;

    while (running) {
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

        chatDemo.Update();

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        chatDemo.RenderChat();

        ImGui::Render();
        SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    curl_global_cleanup();

    std::cout << "Chat demo shutdown completed." << std::endl;
    return 0;
}
