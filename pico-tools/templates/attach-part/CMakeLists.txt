cmake_minimum_required(VERSION 3.13)

# Include pico-sdk via environment variable (NO relative paths)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(PROJECT_NAME)

# Initialize the SDK
pico_sdk_init()

# Get Git commit hash for version info
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

# If git command failed, use default
if(NOT GIT_HASH)
    set(GIT_HASH "unknown")
endif()

# Add shared libraries via environment variables
add_subdirectory($ENV{LIBRARIES_PATH}/console_logger console_logger)

# Create the executable
add_executable(PROJECT_NAME
    main.cpp
)

# Add version definitions
target_compile_definitions(PROJECT_NAME PRIVATE
    GIT_HASH="${GIT_HASH}"
)

# Configure stdio
pico_enable_stdio_usb(PROJECT_NAME 1)
pico_enable_stdio_uart(PROJECT_NAME 0)

# Link required libraries
target_link_libraries(PROJECT_NAME
    pico_stdlib
    hardware_gpio
    hardware_timer
    hardware_watchdog
    console_logger
)

# Create outputs
pico_add_extra_outputs(PROJECT_NAME)