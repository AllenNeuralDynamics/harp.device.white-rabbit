cmake_minimum_required(VERSION 3.13)

add_definitions(-DDEBUG) # Uncomment for debugging

#set(GIT_HASH, "\"$(shell git rev-parse HEAD)\"")
#set(GIT_HASH, "\"$(printf "[%s, %s, %s, %s, %s, %s, %s]" `git rev-parse --short HEAD | sed  's/\\(.\\)/\\1 /g'`)\"")
#add_compile_definitions(GIT_HASH="${GIT_HASH}")
#message(WARNING "Computed Git Hash: ${GIT_HASH}")

# PICO_SDK_PATH must be defined.
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

# Use modern conventions like std::invoke
set(CMAKE_CXX_STANDARD 17)

project(white_rabbit)

pico_sdk_init()
add_subdirectory(lib/harp.core.rp2040/firmware) # Path to harp.core.rp2040.

add_library(uart_nonblocking
    src/uart_nonblocking.cpp
)

add_library(white_rabbit_app
    src/white_rabbit_app.cpp
)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

include_directories(inc)

target_link_libraries(uart_nonblocking hardware_dma hardware_timer pico_stdlib)
target_link_libraries(white_rabbit_app harp_core harp_c_app harp_sync
                      hardware_divider pico_stdlib)

target_link_libraries(${PROJECT_NAME} harp_core harp_c_app harp_sync pico_stdlib
                      hardware_dma hardware_timer uart_nonblocking
                      white_rabbit_app)

pico_add_extra_outputs(${PROJECT_NAME})

if(DEBUG)
    message(WARNING "Debug printf() messages from harp core to UART with baud \
            rate 921600.")
    pico_enable_stdio_uart(${PROJECT_NAME} 1) # UART stdio for printf.
    # Additional libraries need to have stdio init also.
endif()

