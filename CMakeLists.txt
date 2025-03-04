cmake_minimum_required(VERSION 3.10)

project(SimpleRTOS C)

# Set the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include headers from the include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Create a static library from RTOS modules.
add_library(rtos STATIC
    src/rtos_kernel.c
    src/rtos_semaphore.c
    src/memory_alloc.c
    src/callback_mq.c
)

# Build the executable using main.c
add_executable(SimpleRTOS
    src/main.c
)

# Link the RTOS library to the executable.
target_link_libraries(SimpleRTOS rtos)
