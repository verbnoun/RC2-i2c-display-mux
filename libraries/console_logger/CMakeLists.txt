cmake_minimum_required(VERSION 3.13)

# console_logger library
add_library(console_logger 
    console_logger.cpp
)

target_include_directories(console_logger PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link to Pico SDK
target_link_libraries(console_logger
    pico_stdlib
    pico_time
    pico_unique_id
    hardware_watchdog
)