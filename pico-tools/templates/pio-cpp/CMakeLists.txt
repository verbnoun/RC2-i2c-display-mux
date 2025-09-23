cmake_minimum_required(VERSION 3.13)

# Include pico-sdk via environment variable (NO relative paths)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(PROJECT_NAME)

# Initialize the SDK
pico_sdk_init()

# Add shared libraries via environment variables as needed:
# add_subdirectory($ENV{LIBRARIES_PATH}/console_logger console_logger)
# add_subdirectory($ENV{LIBRARIES_PATH}/pot_scanner pot_scanner)

# Generate PIO headers
pico_generate_pio_header(PROJECT_NAME ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)
pico_generate_pio_header(PROJECT_NAME ${CMAKE_CURRENT_LIST_DIR}/quadrature_encoder.pio)
pico_generate_pio_header(PROJECT_NAME ${CMAKE_CURRENT_LIST_DIR}/uart_tx.pio)

# Create the executable
add_executable(PROJECT_NAME
    main.cpp
    ws2812_driver.cpp
    encoder_driver.cpp
    uart_pio_driver.cpp
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(PROJECT_NAME 1)
pico_enable_stdio_uart(PROJECT_NAME 0)

# Link required libraries
target_link_libraries(PROJECT_NAME
    pico_stdlib
    hardware_pio
    hardware_gpio
    hardware_timer
    hardware_clocks
    hardware_irq
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(PROJECT_NAME)