#============================================================================
# SH1107 OLED Graphics Library - CMakeLists.txt
# Modular Adafruit GFX + SH1107 library for Pico SDK
#============================================================================

cmake_minimum_required(VERSION 3.13)

# Create library
add_library(oled_sh1107_gfx STATIC
    # Core graphics library
    src/Adafruit_GFX.cpp
    
    # SH110X base class
    src/Adafruit_SH110X.cpp
    
    # SH1107-specific driver
    src/Adafruit_SH1107.cpp
)

# Include directories
target_include_directories(oled_sh1107_gfx PUBLIC
    include
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Link required Pico SDK libraries
target_link_libraries(oled_sh1107_gfx
    pico_stdlib
    hardware_spi
    hardware_gpio
)

# Enable C++ standard library features
target_compile_features(oled_sh1107_gfx PUBLIC cxx_std_17)

# Compile definitions for optimization
target_compile_definitions(oled_sh1107_gfx PUBLIC
    PICO_STDIO_ENABLE_CRLF_SUPPORT=0  # Reduce code size if not needed
    PICO_STDIO_DEFAULT_CRLF=0
)

#============================================================================
# LIBRARY INFORMATION
#============================================================================

# Set library properties
set_target_properties(oled_sh1107_gfx PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    DESCRIPTION "Modular SH1107 OLED Graphics Library for Raspberry Pi Pico"
)

# Optional: Print configuration info
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building oled_sh1107_gfx library in Debug mode")
    target_compile_definitions(oled_sh1107_gfx PUBLIC DEBUG=1)
endif()

#============================================================================
# USAGE EXAMPLE IN PARENT CMakeLists.txt:
# 
# # Add the library
# add_subdirectory($ENV{LIBRARIES_PATH}/oled_sh1107_gfx oled_sh1107_gfx)
# 
# # Link to your project
# target_link_libraries(your_project 
#     pico_stdlib
#     oled_sh1107_gfx
# )
#============================================================================