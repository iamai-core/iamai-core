# Core
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(GGML_CUDA OFF CACHE BOOL "Disable CUDA" FORCE)
set(WHISPER_OPENBLAS OFF CACHE BOOL "disable OpenBLAS" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/llama.cpp ${CMAKE_BINARY_DIR}/llama.cpp)
add_subdirectory(${CMAKE_SOURCE_DIR}/whisper.cpp ${CMAKE_BINARY_DIR}/whisper.cpp-build)

# Find required Windows libraries only on Windows
if(WIN32)
    find_library(SHELL32_LIBRARY Shell32 REQUIRED)
    find_library(OLE32_LIBRARY Ole32n REQUIRED)
    set(PLATFORM_LIBS ${SHELL32_LIBRARY} ${OLE32_LIBRARY})
endif()

# Main executable
add_executable(iamai-core
    main.cpp
    interface.cpp
    folder_manager.cpp
    whisper_interface.cpp
    whisper_interface.h
    # Only include Windows resource file on Windows
    $<$<BOOL:${WIN32}>:win.rc>
)

target_include_directories(iamai-core PRIVATE
    ${CMAKE_SOURCE_DIR}/llama.cpp
    ${CMAKE_SOURCE_DIR}/whisper.cpp
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries including platform-specific
target_link_libraries(iamai-core PRIVATE
    whisper
    llama
    ${PLATFORM_LIBS}
)

# Main dll/so/dylib
add_library(iamai-core-lib SHARED
    iamai-core-lib.cpp
    interface.cpp
    folder_manager.cpp
)

set_target_properties(iamai-core-lib PROPERTIES
    OUTPUT_NAME "iamai-core"
    PREFIX ""
)

target_include_directories(iamai-core-lib PRIVATE
    ${CMAKE_SOURCE_DIR}/llama.cpp
)


# Link libraries conditionally
target_link_libraries(iamai-core-lib PRIVATE
    llama
    ${PLATFORM_LIBS}
)

# Add platform-specific compile definitions
if(WIN32)
    target_compile_definitions(iamai-core PRIVATE _WIN32)
    target_compile_definitions(iamai-core-lib PRIVATE _WIN32)
else()
    # For macOS, if needed
    if(APPLE)
        target_compile_definitions(iamai-core PRIVATE __APPLE__)
        target_compile_definitions(iamai-core-lib PRIVATE __APPLE__)
    endif()
endif()