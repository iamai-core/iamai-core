cmake_minimum_required(VERSION 3.15)
project(test C CXX)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(llama.cpp)

add_executable(${PROJECT_NAME} test.cpp interface.cpp)

target_link_libraries(${PROJECT_NAME} llama)
