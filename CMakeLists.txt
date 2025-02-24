cmake_minimum_required(VERSION 3.15)
project(iamai-core C CXX)

# Set C/C++ standards before anything else
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(core)
#add_subdirectory(test-whisper)
#add_subdirectory(test-ggml)
#add_subdirectory(test-llmc)
