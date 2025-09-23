cmake_minimum_required(VERSION 3.13)

# Activity LED Library
add_library(activity_led STATIC
    activity_led.cpp
    activity_led.h
)

# Target includes
target_include_directories(activity_led PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link required libraries
target_link_libraries(activity_led
    pico_stdlib
    hardware_gpio
    console_logger
)