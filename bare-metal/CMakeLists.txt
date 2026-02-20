cmake_minimum_required(VERSION 3.10)
project(rtos-bare-metal-tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

enable_testing()

# Unity test framework (vendored in tests/unity/)
set(UNITY_DIR ${PROJECT_SOURCE_DIR}/tests/unity/src)
add_library(unity STATIC ${UNITY_DIR}/unity.c)
target_include_directories(unity PUBLIC ${UNITY_DIR})

include_directories(${PROJECT_SOURCE_DIR}/include)

# --- Memory allocator tests ---
add_executable(test_mem
    tests/test_mem.c
    kernel/mem.c
)
target_link_libraries(test_mem unity)
add_test(NAME test_mem COMMAND test_mem)
