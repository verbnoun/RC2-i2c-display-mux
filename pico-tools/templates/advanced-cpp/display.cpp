#include "display.h"
#include <stdio.h>
#include <string.h>
#include "hardware/i2c.h"
#include "hardware/gpio.h"

// Display configuration (example for SSD1306 OLED)
#define DISPLAY_I2C i2c1
#define DISPLAY_ADDR 0x3C
#define DISPLAY_SDA 6
#define DISPLAY_SCL 7

static bool display_available = false;

void display_init() {
    // Initialize I2C for display
    i2c_init(DISPLAY_I2C, 400000); // 400kHz
    gpio_set_function(DISPLAY_SDA, GPIO_FUNC_I2C);
    gpio_set_function(DISPLAY_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(DISPLAY_SDA);
    gpio_pull_up(DISPLAY_SCL);
    
    // Test if display is connected
    uint8_t test_data = 0x00;
    int result = i2c_write_blocking(DISPLAY_I2C, DISPLAY_ADDR, &test_data, 1, false);
    
    if (result >= 0) {
        display_available = true;
        printf("Display connected at 0x%02X\n", DISPLAY_ADDR);
        
        // Initialize display (basic commands for SSD1306)
        uint8_t init_commands[] = {
            0x00, 0xAE, // Display off
            0x00, 0xD5, 0x00, 0x80, // Set display clock divide
            0x00, 0xA8, 0x00, 0x3F, // Set multiplex ratio
            0x00, 0xD3, 0x00, 0x00, // Set display offset
            0x00, 0x40, // Set start line
            0x00, 0x8D, 0x00, 0x14, // Charge pump
            0x00, 0x20, 0x00, 0x00, // Memory mode
            0x00, 0xA1, // Set segment re-map
            0x00, 0xC8, // Set COM output scan direction
            0x00, 0xDA, 0x00, 0x12, // Set COM pins
            0x00, 0x81, 0x00, 0xCF, // Set contrast
            0x00, 0xD9, 0x00, 0xF1, // Set pre-charge
            0x00, 0xDB, 0x00, 0x40, // Set VCOM detect
            0x00, 0xA4, // Entire display on
            0x00, 0xA6, // Set normal display
            0x00, 0xAF  // Display on
        };
        
        for (size_t i = 0; i < sizeof(init_commands); i += 2) {
            i2c_write_blocking(DISPLAY_I2C, DISPLAY_ADDR, &init_commands[i], 2, false);
            sleep_ms(1);
        }
        
        display_clear();
    } else {
        printf("No display found at 0x%02X\n", DISPLAY_ADDR);
    }
}

void display_update_demo(float temperature, uint16_t light_level) {
    if (!display_available) {
        printf("Display Demo: Temp=%.1f°C, Light=%d\n", temperature, light_level);
        return;
    }
    
    // This is a simplified demo - real implementation would need
    // proper font rendering and display buffer management
    
    printf("Updating display: Temp=%.1f°C, Light=%d\n", temperature, light_level);
    
    // Example of sending display data
    static uint32_t frame_count = 0;
    frame_count++;
    
    // Send some pattern data (placeholder)
    uint8_t display_data[129];
    display_data[0] = 0x40; // Data mode
    
    for (int i = 1; i < 129; i++) {
        // Create a simple pattern that changes over time
        display_data[i] = (uint8_t)((i + frame_count) & 0xFF);
    }
    
    i2c_write_blocking(DISPLAY_I2C, DISPLAY_ADDR, display_data, 129, false);
}

void display_clear() {
    if (!display_available) return;
    
    // Clear display buffer
    uint8_t clear_data[129];
    clear_data[0] = 0x40; // Data mode
    memset(&clear_data[1], 0x00, 128);
    
    // Send clear data to all pages
    for (int page = 0; page < 8; page++) {
        // Set page address
        uint8_t page_cmd[] = {0x00, (uint8_t)(0xB0 + page)};
        i2c_write_blocking(DISPLAY_I2C, DISPLAY_ADDR, page_cmd, 2, false);
        
        // Set column address
        uint8_t col_cmd[] = {0x00, 0x00, 0x00, 0x10};
        i2c_write_blocking(DISPLAY_I2C, DISPLAY_ADDR, col_cmd, 4, false);
        
        // Send clear data
        i2c_write_blocking(DISPLAY_I2C, DISPLAY_ADDR, clear_data, 129, false);
    }
}

void display_print(const char* text) {
    if (!display_available) {
        printf("Display: %s\n", text);
        return;
    }
    
    // Placeholder for text rendering
    // Real implementation would convert text to bitmap and send to display
    printf("Display Print: %s\n", text);
}

void display_set_cursor(uint8_t x, uint8_t y) {
    if (!display_available) return;
    
    // Set cursor position on display
    uint8_t cursor_cmd[] = {
        0x00, (uint8_t)(0xB0 + y), // Set page
        0x00, (uint8_t)(x & 0x0F), // Set lower column
        0x00, (uint8_t)(0x10 + (x >> 4)) // Set upper column
    };
    
    i2c_write_blocking(DISPLAY_I2C, DISPLAY_ADDR, cursor_cmd, 6, false);
}