# llm.c

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin-llmc)

#add_subdirectory(${CMAKE_SOURCE_DIR}/llm.c ${CMAKE_BINARY_DIR}/llm.c)

# Test executable
#add_executable(test-llmc test.cpp interface.cpp)
#target_link_libraries(test-llmc PRIVATE llmc)




# Add CUDA support
find_package(CUDA REQUIRED)
enable_language(CUDA)

# Set CUDA architectures based on available GPUs
set(CMAKE_CUDA_ARCHITECTURES 70 75 80 86 89 90)

# Create llmc library from source files
add_library(llmc STATIC
    ${CMAKE_SOURCE_DIR}/llmc/train_gpt2.cu
    ${CMAKE_SOURCE_DIR}/llmc/test_gpt2.cu
)

# Set compile definitions for llmc
target_compile_definitions(llmc PRIVATE
    ENABLE_BF16  # Default to bfloat16 mode like original Makefile
)

# Set include directories for llmc
target_include_directories(llmc PUBLIC
    ${CMAKE_SOURCE_DIR}/llmc
    ${CUDA_INCLUDE_DIRS}
)

# Set CUDA compiler flags similar to original Makefile
target_compile_options(llmc PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:
        --use_fast_math
        -O3
        --threads 0
    >
)

# Add CUDA libraries
target_link_libraries(llmc PUBLIC
    ${CUDA_cublas_LIBRARY}
    ${CUDA_cublasLt_LIBRARY}
)

# Test executable
add_executable(test-llmc test.cpp interface.cpp)

target_link_libraries(test-llmc PRIVATE llmc)
