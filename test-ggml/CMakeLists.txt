cmake_minimum_required(VERSION 3.10)
project(aapl_prediction)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files for the project
set(SOURCES
    src/model.cpp
    src/data.cpp
    src/main.cpp
)

# Create executable
add_executable(aapl_prediction ${SOURCES})

# Set up compiler flags
if(MSVC)
    target_compile_options(aapl_prediction PRIVATE /W4 /Zc:preprocessor)
else()
    target_compile_options(aapl_prediction PRIVATE -Wall -Wextra)
endif()

# Ensure the output directory exists
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)