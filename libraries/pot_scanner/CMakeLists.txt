# Pot Scanner Library - CD74HC4067 + Potentiometer Management
# Part of SoftwareC modular library ecosystem

cmake_minimum_required(VERSION 3.13)

# Create pot_scanner library
add_library(pot_scanner
    pot_scanner.cpp
    pot_scanner.h
)

# Link required libraries
# Note: console_logger dependency is provided by parent project
target_link_libraries(pot_scanner
    pico_stdlib
    hardware_gpio
    hardware_adc
    console_logger
)

# Make headers available to projects that link this library
target_include_directories(pot_scanner PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)