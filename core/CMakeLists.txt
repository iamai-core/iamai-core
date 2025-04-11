# Core
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(GGML_CUDA OFF CACHE BOOL "Disable CUDA" FORCE)
set(WHISPER_OPENBLAS OFF CACHE BOOL "disable OpenBLAS" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/llama.cpp ${CMAKE_BINARY_DIR}/llama.cpp)
add_subdirectory(${CMAKE_SOURCE_DIR}/whisper.cpp ${CMAKE_BINARY_DIR}/whisper.cpp-build)

# Find required Windows libraries
find_library(SHELL32_LIBRARY Shell32)
find_library(OLE32_LIBRARY Ole32)

# Main executable
add_executable(iamai-core
    main.cpp
    interface.cpp
    folder_manager.cpp
    whisper_interface.cpp
    whisper_interface.h
    win.rc
)

target_include_directories(iamai-core PRIVATE
    ${CMAKE_SOURCE_DIR}/llama.cpp
    ${CMAKE_SOURCE_DIR}/whisper.cpp
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(iamai-core PRIVATE
    whisper
    llama
    ${SHELL32_LIBRARY}
    ${OLE32_LIBRARY}
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

target_link_libraries(iamai-core-lib PRIVATE
    llama
    ${SHELL32_LIBRARY}
    ${OLE32_LIBRARY}
)

# whisper-interface DLL
add_library(whisper-interface-lib SHARED
    whisper-interface-lib.cpp
    whisper_interface.cpp
    folder_manager.cpp
)

set_target_properties(whisper-interface-lib PROPERTIES
    OUTPUT_NAME "whisper-interface"
    PREFIX ""
)

target_include_directories(whisper-interface-lib PRIVATE
    ${CMAKE_SOURCE_DIR}/whisper.cpp
)

target_link_libraries(whisper-interface-lib PRIVATE
    whisper
    ${SHELL32_LIBRARY}
    ${OLE32_LIBRARY}
)