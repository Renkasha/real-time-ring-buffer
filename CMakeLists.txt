cmake_minimum_required(VERSION 3.12)
project(RealTimeRingBuffer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Example executable
add_executable(ring_buffer_example src/examples/ring_buffer_example.cpp)
target_link_libraries(ring_buffer_example PRIVATE pthread)
set_target_properties(ring_buffer_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Benchmark executable
add_executable(ring_buffer_benchmark src/benchmark/ring_buffer_benchmark.cpp)
target_link_libraries(ring_buffer_benchmark PRIVATE pthread)
set_target_properties(ring_buffer_benchmark PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Google Test
include(FetchContent)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Tests
add_executable(ring_buffer_test src/tests/ring_buffer_test.cpp)
target_link_libraries(ring_buffer_test PRIVATE gtest gtest_main pthread)
set_target_properties(ring_buffer_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

enable_testing()
add_test(NAME RingBufferTests COMMAND ring_buffer_test)
