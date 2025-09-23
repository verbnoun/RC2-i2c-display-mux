cmake_minimum_required(VERSION 3.13)

# I/O Resource Management Library
# Universal traffic controller for all I/O expansion (multiplexers, switches, expanders)

add_library(io_manager
    io_manager.cpp
    drivers/analog_mux_driver.cpp
    drivers/i2c_switch_driver.cpp
)

target_include_directories(io_manager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(io_manager
    pico_stdlib
    hardware_gpio
    hardware_i2c
    hardware_timer
    pico_sync
    console_logger
)

# Enable C++17 for modern features
target_compile_features(io_manager PUBLIC cxx_std_17)