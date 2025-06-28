# Core

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# # Copy CUDA DLLs from CUDA installation directory to output directory
# set(CUDA_DLL_VERSION "12")
# add_custom_target(copy_cuda_dlls ALL
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         "$ENV{CUDA_PATH}/bin/cudart64_${CUDA_DLL_VERSION}.dll"
#         "$ENV{CUDA_PATH}/bin/cublas64_${CUDA_DLL_VERSION}.dll"
#         "$ENV{CUDA_PATH}/bin/cublasLt64_${CUDA_DLL_VERSION}.dll"
#         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
#     COMMENT "Copying CUDA DLLs to bin/${CONFIG} directory"
# )

# Enable acceleration
# option(GGML_CUDA                            "ggml: use CUDA"                                  ON)
# option(GGML_CUDA_F16                        "ggml: use 16 bit floats for some calculations"   ON)
# option(GGML_METAL                           "ggml: use Metal"                                 ON)
# option(GGML_METAL_USE_BF16                  "ggml: use bfloat if available"                   ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/llama.cpp ${CMAKE_BINARY_DIR}/llama.cpp)

# # Main executable
# add_executable(iamai-core iamai-core.cpp interface.cpp win.rc)
# target_link_libraries(iamai-core PRIVATE llama)

# # Main dll/so/dylib
# add_library(iamai-core-lib SHARED iamai-core-lib.cpp interface.cpp)
# set_target_properties(iamai-core-lib PROPERTIES
#     OUTPUT_NAME "iamai-core"
#     PREFIX ""
# )
# target_link_libraries(iamai-core-lib PRIVATE llama)

# Test executables
add_executable(test-interface test-interface.cpp interface.cpp win.rc)
target_link_libraries(test-interface PRIVATE llama)
