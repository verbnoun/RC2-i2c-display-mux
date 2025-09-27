# SSD1306 Graphics Library for Raspberry Pi Pico
# High-level graphics and text rendering for SSD1306 OLED displays

add_library(ssd1306_graphics STATIC
    ssd1306_graphics.c
    japanese_char_map.c
    oled_display_manager.c
    fonts/misaki_8px_japanese.c
    fonts/pixelmplus_10px_japanese.c
    fonts/pixelmplus_12px_japanese.c
)

target_include_directories(ssd1306_graphics PUBLIC .)

target_link_libraries(ssd1306_graphics
    pico_stdlib
    hardware_i2c
)