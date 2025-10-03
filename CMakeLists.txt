cmake_minimum_required(VERSION 3.18)
project(iamai-core C CXX)

# Set C/C++ standards before anything else
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static link MSVC runtime
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Then choose which subdirectory to build
add_subdirectory(core core-build)
# add_subdirectory(chat-demo chat-demo-build)

# add_subdirectory(test-whisper)
# add_subdirectory(test-ggml)
# add_subdirectory(test-llmc)
