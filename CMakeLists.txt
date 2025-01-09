cmake_minimum_required(VERSION 3.15)
project(iamai-core C CXX)

# Set C/C++ standards before anything else
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Disable all CPU-specific optimizations
set(LLAMA_NATIVE OFF CACHE BOOL "disable CPU optimizations" FORCE)
set(GGML_NO_ACCELERATE ON CACHE BOOL "disable Accelerate framework" FORCE)
set(GGML_CPU_FEATURES_DISABLE ON CACHE BOOL "disable CPU features" FORCE)

# Enable CUDA but disable other acceleration
set(GGML_CUDA ON CACHE BOOL "Enable CUDA" FORCE)
set(GGML_METAL OFF CACHE BOOL "disable Metal" FORCE)
set(GGML_OPENBLAS OFF CACHE BOOL "disable OpenBLAS" FORCE)

# Add llama.cpp subdirectory
add_subdirectory(llama.cpp)

# Main executable
add_executable(iamai-core iamai-core.cpp interface.cpp)
target_link_libraries(iamai-core PRIVATE llama)

# Main dll/so/dylib
add_library(iamai-core-lib SHARED iamai-core-lib.cpp interface.cpp)
set_target_properties(iamai-core-lib PROPERTIES
    OUTPUT_NAME "iamai-core"
    PREFIX ""
)
target_link_libraries(iamai-core-lib PRIVATE llama)

# Test executable
add_executable(test test.cpp interface.cpp)
target_link_libraries(test PRIVATE llama)
