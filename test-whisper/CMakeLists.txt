# whisper.cpp

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin-whisper)

add_subdirectory(${CMAKE_SOURCE_DIR}/whisper.cpp ${CMAKE_BINARY_DIR}/whisper.cpp)

# Test executable
add_executable(test-whisper test.cpp interface.cpp)
target_link_libraries(test-whisper PRIVATE whisper)
