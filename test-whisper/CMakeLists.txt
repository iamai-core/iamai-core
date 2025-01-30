# whisper.cpp setup
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Copy CUDA DLLs
set(CUDA_DLL_VERSION "12")
add_custom_target(copy_cuda_dlls_whisper ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$ENV{CUDA_PATH}/bin/cudart64_${CUDA_DLL_VERSION}.dll"
        "$ENV{CUDA_PATH}/bin/cublas64_${CUDA_DLL_VERSION}.dll"
        "$ENV{CUDA_PATH}/bin/cublasLt64_${CUDA_DLL_VERSION}.dll"
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
    COMMENT "Copying CUDA DLLs to bin/${CONFIG} directory for Whisper"
)

# Enable CUDA but disable other acceleration for whisper
set(GGML_CUDA ON CACHE BOOL "Enable CUDA" FORCE)
set(WHISPER_OPENBLAS OFF CACHE BOOL "disable OpenBLAS" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/llama.cpp ${CMAKE_BINARY_DIR}/llama.cpp)

# Add whisper.cpp as subdirectory
add_subdirectory(${CMAKE_SOURCE_DIR}/whisper.cpp ${CMAKE_BINARY_DIR}/whisper.cpp-build)

# Find required Windows libraries
find_library(SHELL32_LIBRARY Shell32)
find_library(OLE32_LIBRARY Ole32)

# Main executable
add_executable(test-whisper
    test.cpp
    whisper_interface.cpp
    whisper_interface.h
)

# Set include directories explicitly for test-whisper target
target_include_directories(test-whisper PRIVATE
    ${CMAKE_SOURCE_DIR}/whisper.cpp
    ${CMAKE_SOURCE_DIR}/llama.cpp
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(test-whisper PRIVATE
    whisper
    llama
    ${SHELL32_LIBRARY}
    ${OLE32_LIBRARY}
)