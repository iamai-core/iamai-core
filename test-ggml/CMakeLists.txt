# ggml

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin-ggml)

add_subdirectory(${CMAKE_SOURCE_DIR}/ggml ${CMAKE_BINARY_DIR}/ggml)

# Test executable
add_executable(test-ggml test.cpp interface.cpp)
target_link_libraries(test-ggml PRIVATE ggml)
