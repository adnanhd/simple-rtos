cmake_minimum_required(VERSION 3.10)

project(SimpleRTOS C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(rtos STATIC
    src/rtos_kernel.c
    src/rtos_semaphore.c
    src/mem.c
    src/mq.c
    src/ipc.c
    src/net.c
)

add_executable(SimpleRTOS
    src/main.c
)

target_link_libraries(SimpleRTOS rtos)
