cmake_minimum_required(VERSION 3.15)
project(iamai-core C CXX)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(llama.cpp)

add_executable(iamai-core iamai-core.cpp interface.cpp)
target_link_libraries(iamai-core llama)

add_executable(test test.cpp interface.cpp)
target_link_libraries(test llama)
