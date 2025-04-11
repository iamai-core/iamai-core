cmake_minimum_required(VERSION 3.15)

# ggml setup - using relative path to parent directory's ggml
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin-ggml)
add_subdirectory(${CMAKE_SOURCE_DIR}/ggml ${CMAKE_BINARY_DIR}/ggml)

# Add the library targets
add_library(m4_dataset STATIC
    m4_dataset.cpp
    m4_dataset.h
)

add_library(timeseries_common STATIC
    timeseries_common.cpp
    timeseries_common.h
)

# Test targets
add_executable(test-ggml 
    main.cpp 
    interface.cpp
    interface.h
)

add_executable(train_monthly
    train_monthly.cpp
    interface.cpp
    interface.h
)

add_executable(eval_monthly
    eval_monthly.cpp
    interface.cpp
    interface.h
)

# Set up includes for m4_dataset
target_include_directories(m4_dataset PUBLIC
    ${CMAKE_SOURCE_DIR}/ggml/include
    ${CMAKE_SOURCE_DIR}/ggml/examples
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set up includes for timeseries_common
target_include_directories(timeseries_common PUBLIC
    ${CMAKE_SOURCE_DIR}/ggml/include
    ${CMAKE_SOURCE_DIR}/ggml/examples
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries for m4_dataset
target_link_libraries(m4_dataset PUBLIC ggml)

# Link libraries for timeseries_common
target_link_libraries(timeseries_common PUBLIC ggml)

# Link executables with all required libraries
foreach(target test-ggml train_monthly eval_monthly)
    target_include_directories(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/ggml/include
        ${CMAKE_SOURCE_DIR}/ggml/examples
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_libraries(${target} PRIVATE
        ggml
        m4_dataset
        timeseries_common
    )
endforeach()