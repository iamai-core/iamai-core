# Chat Demo

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(BUILD_SHARED_LIBS "build shared libraries" OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/llama.cpp ${CMAKE_BINARY_DIR}/llama.cpp-build)

option(SDL_TEST_LIBRARY "Build the SDL3_test library" OFF)
option(SDL_SHARED "Build SDL3 shared library" OFF)
option(SDL_STATIC "Build SDL3 static library" ON)
add_subdirectory(SDL SDL-build)

# libcurl configuration - disable unnecessary features
set(BUILD_CURL_EXE OFF CACHE BOOL "Build curl executable" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "Build static libraries" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Build tests" FORCE)
set(ENABLE_CURL_MANUAL OFF CACHE BOOL "Build curl manual" FORCE)
set(BUILD_LIBCURL_DOCS OFF CACHE BOOL "Build libcurl docs" FORCE)
set(CURL_ENABLE_EXPORT_TARGET OFF CACHE BOOL "Enable export target" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
set(CURL_DISABLE_INSTALL ON CACHE BOOL "Disable installation targets" FORCE)
# Force disable libpsl and cookies
set(CURL_USE_LIBPSL OFF CACHE BOOL "Use libpsl for cookie domain checks" FORCE)
set(CURL_DISABLE_COOKIES ON CACHE BOOL "Disable cookies support" FORCE)
# Disable protocols you don't need (but enable the ones you want)
set(CURL_DISABLE_LDAP ON CACHE BOOL "Disable LDAP support" FORCE)
set(CURL_DISABLE_LDAPS ON CACHE BOOL "Disable LDAPS support" FORCE)
set(CURL_DISABLE_RTSP ON CACHE BOOL "Disable RTSP support" FORCE)
set(CURL_DISABLE_DICT ON CACHE BOOL "Disable DICT support" FORCE)
set(CURL_DISABLE_FILE ON CACHE BOOL "Disable FILE support" FORCE)
set(CURL_DISABLE_GOPHER ON CACHE BOOL "Disable GOPHER support" FORCE)
set(CURL_DISABLE_IMAP ON CACHE BOOL "Disable IMAP support" FORCE)
set(CURL_DISABLE_MQTT ON CACHE BOOL "Disable MQTT support" FORCE)
set(CURL_DISABLE_POP3 ON CACHE BOOL "Disable POP3 support" FORCE)
set(CURL_DISABLE_SMB ON CACHE BOOL "Disable SMB support" FORCE)
set(CURL_DISABLE_SMTP ON CACHE BOOL "Disable SMTP support" FORCE)
set(CURL_DISABLE_TELNET ON CACHE BOOL "Disable TELNET support" FORCE)
set(CURL_DISABLE_TFTP ON CACHE BOOL "Disable TFTP support" FORCE)
set(CURL_DISABLE_IPFS ON CACHE BOOL "Disable IPFS support" FORCE)
# Enable the protocols you want: HTTP, HTTPS, FTP, WS, WSS
set(CURL_DISABLE_HTTP OFF CACHE BOOL "Enable HTTP support" FORCE)
set(CURL_DISABLE_FTP OFF CACHE BOOL "Enable FTP support" FORCE)
set(CURL_DISABLE_WEBSOCKETS OFF CACHE BOOL "Enable WebSocket support" FORCE)
# Disable other optional features to reduce dependencies
set(CURL_DISABLE_ALTSVC ON CACHE BOOL "Disable alt-svc support" FORCE)
set(CURL_DISABLE_HSTS ON CACHE BOOL "Disable HSTS support" FORCE)
set(CURL_DISABLE_CRYPTO_AUTH ON CACHE BOOL "Disable crypto auth" FORCE)
set(CURL_DISABLE_NTLM ON CACHE BOOL "Disable NTLM support" FORCE)
set(CURL_DISABLE_NEGOTIATE_AUTH ON CACHE BOOL "Disable negotiate authentication" FORCE)
set(CURL_DISABLE_KERBEROS_AUTH ON CACHE BOOL "Disable Kerberos authentication" FORCE)
# set(CURL_DISABLE_VERBOSE_STRINGS ON CACHE BOOL "Disable verbose strings" FORCE)
# SSL/TLS settings - Cross-platform SSL support
if(WIN32)
    # Use Windows Schannel on Windows
    set(CURL_USE_SCHANNEL ON CACHE BOOL "Use Windows Schannel" FORCE)
    set(CURL_USE_OPENSSL OFF CACHE BOOL "Use OpenSSL" FORCE)
else()
    # Use OpenSSL on Linux and other platforms
    set(CURL_USE_SCHANNEL OFF CACHE BOOL "Use Windows Schannel" FORCE)
    set(CURL_USE_OPENSSL ON CACHE BOOL "Use OpenSSL" FORCE)
endif()
add_subdirectory(curl curl-build)


# Create ImGui library target
add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_sdl3.cpp
    imgui/backends/imgui_impl_sdlrenderer3.cpp
)
# Set ImGui include directories
target_include_directories(imgui PUBLIC
    imgui
    imgui/backends
)
# Link ImGui with SDL3
target_link_libraries(imgui PUBLIC SDL3::SDL3)
# For 32bit Unicode fonts
target_compile_definitions(imgui PUBLIC IMGUI_USE_WCHAR32)

# Chat demo executable
add_executable(chat-demo
    main.cpp
    chat_demo.cpp
    chat_demo_ui.cpp
    settings_manager.cpp
    ${CMAKE_SOURCE_DIR}/core/folder_manager.cpp
    ${CMAKE_SOURCE_DIR}/core/model_manager.cpp
    ${CMAKE_SOURCE_DIR}/core/interface.cpp
    win.rc
)
# Set manifest file for Windows
if(WIN32)
    set_target_properties(chat-demo PROPERTIES
        # Set to TRUE to remove console window
        WIN32_EXECUTABLE FALSE
        LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:${CMAKE_CURRENT_SOURCE_DIR}/app.manifest"
    )
endif()
target_link_libraries(chat-demo PRIVATE
    llama
    SDL3::SDL3
    imgui
    libcurl
)
