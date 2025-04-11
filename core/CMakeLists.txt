# Core

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Copy CUDA DLLs from CUDA installation directory to output directory
set(CUDA_DLL_VERSION "12")
add_custom_target(copy_cuda_dlls ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$ENV{CUDA_PATH}/bin/cudart64_${CUDA_DLL_VERSION}.dll"
        "$ENV{CUDA_PATH}/bin/cublas64_${CUDA_DLL_VERSION}.dll"
        "$ENV{CUDA_PATH}/bin/cublasLt64_${CUDA_DLL_VERSION}.dll"
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
    COMMENT "Copying CUDA DLLs to bin/${CONFIG} directory"
)


# Disable all CPU-specific optimizations
#set(LLAMA_NATIVE OFF CACHE BOOL "disable CPU optimizations" FORCE)
#set(GGML_NO_ACCELERATE ON CACHE BOOL "disable Accelerate framework" FORCE)
#set(GGML_CPU_FEATURES_DISABLE ON CACHE BOOL "disable CPU features" FORCE)

# Enable CUDA but disable other acceleration
set(GGML_CUDA ON CACHE BOOL "Enable CUDA" FORCE)
set(GGML_METAL OFF CACHE BOOL "disable Metal" FORCE)
set(GGML_OPENBLAS OFF CACHE BOOL "disable OpenBLAS" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/llama.cpp ${CMAKE_BINARY_DIR}/llama.cpp)

# Main executable
add_executable(iamai-core iamai-core.cpp interface.cpp win.rc)
target_link_libraries(iamai-core PRIVATE llama)

# Main dll/so/dylib
add_library(iamai-core-lib SHARED iamai-core-lib.cpp interface.cpp)
set_target_properties(iamai-core-lib PROPERTIES
    OUTPUT_NAME "iamai-core"
    PREFIX ""
)
target_link_libraries(iamai-core-lib PRIVATE llama)

<<<<<<< Updated upstream
# Main test executable
add_executable(test test.cpp interface.cpp win.rc)
target_link_libraries(test PRIVATE llama)
=======
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
>>>>>>> Stashed changes
