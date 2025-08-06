# Chat Demo

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# # Find OpenGL
# if(WIN32)
#     set(OPENGL_LIBRARIES opengl32)
# elseif(APPLE)
#     find_library(OPENGL_LIBRARIES OpenGL)
# else()
#     find_package(OpenGL REQUIRED)
#     set(OPENGL_LIBRARIES ${OPENGL_gl_LIBRARY})
# endif()

option(BUILD_SHARED_LIBS "build shared libraries" OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/llama.cpp ${CMAKE_BINARY_DIR}/llama.cpp-build)

option(SDL_SHARED "Build SDL3 shared library" OFF)
option(SDL_STATIC "Build SDL3 static library" ON)
# set(SDL_SHARED OFF CACHE BOOL "Build SDL3 shared library" FORCE)
# set(SDL_STATIC ON CACHE BOOL "Build SDL3 static library" FORCE)
add_subdirectory(SDL SDL-build)



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


# Chat demo executable
add_executable(chat-demo
    main.cpp
    ${CMAKE_SOURCE_DIR}/core/interface.cpp
    ${CMAKE_SOURCE_DIR}/core/win.rc
)
target_link_libraries(chat-demo PRIVATE
    llama
    SDL3::SDL3
    imgui
    # ${OPENGL_LIBRARIES}
)
