cmake_minimum_required(VERSION 3.13)
find_package(Git REQUIRED)
execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD OUTPUT_VARIABLE COMMIT_ID OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Computed Git Hash: ${COMMIT_ID}")
add_definitions(-DGIT_HASH="${COMMIT_ID}") # Usable in source code.


#add_definitions(-DDEBUG) # Uncomment for debugging


#set(PICO_PLATFORM rp2040) # rp2040 is the default.

# PICO_SDK_PATH must be defined.
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

# Use modern conventions like std::invoke
set(CMAKE_CXX_STANDARD 17)
# Enable try/catch exception interface.
#set(PICO_CXX_ENABLE_EXCEPTIONS 1)

#set(PICO_COPY_TO_RAM 1)

project(white_rabbit)

pico_sdk_init()

add_subdirectory(lib/harp.core.rp2040/firmware) # Path to harp.core.rp2040.
add_subdirectory(lib/pico.async-uart) # Path to uart_nonblocking and soft_uart

add_library(white_rabbit_app
    src/white_rabbit_app.cpp
)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

include_directories(inc)

target_link_libraries(white_rabbit_app harp_core harp_c_app harp_sync
                      hardware_divider pico_stdlib uart_nonblocking pio_uart
                      hardware_pio hardware_dma soft_uart)

target_link_libraries(${PROJECT_NAME} harp_core harp_c_app harp_sync pico_stdlib
                      hardware_dma hardware_timer uart_nonblocking pio_uart
                      white_rabbit_app)

pico_add_extra_outputs(${PROJECT_NAME})

if(DEBUG)
    message(WARNING "Debug printf() messages from harp core to UART with baud \
            rate 921600.")
    pico_enable_stdio_uart(${PROJECT_NAME} 1) # UART stdio for printf.
    # Additional libraries need to have stdio init also.
endif()

