//============================================================================
// Adafruit SH110X OLED Display Base Class Implementation - Pico SDK Port
// Base class for SH110X family displays with modular SPI interface
//============================================================================

#include "Adafruit_SH110X.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <string.h>
#include <algorithm>
#include <cstdio>

//============================================================================
// CONSTRUCTOR & DESTRUCTOR
//============================================================================

Adafruit_SH110X::Adafruit_SH110X(const SH1107_Config& config) 
    : Adafruit_GFX(config.display_width, config.display_height),
      config(config),
      buffer(nullptr),
      buffer_allocated(false),
      current_display(0),
      spi_instance(nullptr),
      hardware_initialized(false) {
    
    // Set display-specific defaults (will be overridden by derived classes)
    buffer_size = (config.display_width * config.display_height) / 8;
    page_offset = 0;
    column_offset = 0;
    
    // Get SPI instance
    spi_instance = (config.spi_instance == 0) ? spi0 : spi1;
}

Adafruit_SH110X::~Adafruit_SH110X() {
    if (buffer_allocated && buffer) {
        delete[] buffer;
        buffer = nullptr;
    }
}

//============================================================================
// INITIALIZATION
//============================================================================

bool Adafruit_SH110X::begin() {
    printf("[SH110X] === Starting Graphics Library Initialization ===\n");
    printf("[SH110X] Current state: hardware_initialized=%s, buffer=%p\n", 
           hardware_initialized ? "true" : "false", buffer);
    
    // Validate configuration
    printf("[SH110X] Validating configuration...\n");
    if (!validate_config(config)) {
        printf("[SH110X] ❌ Configuration validation failed!\n");
        return false;
    }
    printf("[SH110X] ✅ Configuration validated\n");
    
    // Initialize hardware
    printf("[SH110X] Initializing SPI hardware...\n");
    if (!init_hardware()) {
        printf("[SH110X] ❌ Hardware initialization failed!\n");
        return false;
    }
    printf("[SH110X] ✅ SPI hardware initialized\n");
    
    // Allocate display buffer if needed
    if (!config.use_external_buffer) {
        printf("[SH110X] Allocating display buffer (%d bytes)...\n", buffer_size);
        buffer = new uint8_t[buffer_size];
        if (!buffer) {
            printf("[SH110X] ❌ Buffer allocation failed!\n");
            return false;
        }
        buffer_allocated = true;
        memset(buffer, 0, buffer_size);
        printf("[SH110X] ✅ Buffer allocated and cleared\n");
    } else {
        buffer = config.external_buffer;
        if (!buffer) {
            printf("[SH110X] ❌ External buffer is null!\n");
            return false;
        }
        printf("[SH110X] ✅ Using external buffer\n");
    }
    
    // Initialize the specific display
    printf("[SH110X] Calling display-specific initialization...\n");
    if (!init_display()) {
        printf("[SH110X] ❌ Display initialization failed!\n");
        return false;
    }
    printf("[SH110X] ✅ Display initialized successfully\n");
    
    // Clear the display buffer (but don't send to hardware yet)
    printf("[SH110X] Clearing display buffer...\n");
    clearDisplay();
    printf("[SH110X] ✅ Display buffer cleared\n");
    
    printf("[SH110X] === About to send buffer to hardware - THIS IS WHERE FLASH OCCURS ===\n");
    printf("[SH110X] Press any key to continue or wait 3 seconds...\n");
    
    // Give user chance to observe before flash
    for (int i = 3; i > 0; i--) {
        printf("[SH110X] Sending buffer in %d seconds...\n", i);
        sleep_ms(1000);
        if (getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) break;
    }
    
    printf("[SH110X] Sending buffer to hardware NOW...\n");
    display();
    printf("[SH110X] ✅ Display buffer sent to hardware\n");
    
    printf("[SH110X] === Graphics Library Initialization Complete ===\n");
    return true;
}

bool Adafruit_SH110X::init_hardware() {
    if (hardware_initialized) {
        printf("[SH110X] Hardware already initialized, skipping\n");
        return true;
    }
    
    // Initialize SPI
    printf("[SH110X] Initializing SPI%d at %d Hz...\n", config.spi_instance, config.spi_frequency);
    spi_init(spi_instance, config.spi_frequency);
    
    // Configure SPI pins
    printf("[SH110X] Configuring SPI pins: SCK=%d, MOSI=%d\n", config.spi_sck_pin, config.spi_mosi_pin);
    gpio_set_function(config.spi_sck_pin, GPIO_FUNC_SPI);
    gpio_set_function(config.spi_mosi_pin, GPIO_FUNC_SPI);
    
    // Configure control pins as outputs
    printf("[SH110X] Configuring control pins: DC=%d, RST=%d, CS=%d\n", 
           config.spi_dc_pin, config.spi_rst_pin, config.spi_cs_pin);
    gpio_init(config.spi_dc_pin);
    gpio_set_dir(config.spi_dc_pin, GPIO_OUT);
    gpio_put(config.spi_dc_pin, true);  // Default to data mode (matches working driver)
    
    gpio_init(config.spi_rst_pin);
    gpio_set_dir(config.spi_rst_pin, GPIO_OUT);
    gpio_put(config.spi_rst_pin, true); // Default to not reset
    
    // Configure CS pin(s)
    gpio_init(config.spi_cs_pin);
    gpio_set_dir(config.spi_cs_pin, GPIO_OUT);
    gpio_put(config.spi_cs_pin, true); // Default to deselected
    
    // Configure additional CS pins for multi-display setups
    if (config.display_count > 1 && config.cs_pins) {
        printf("[SH110X] Configuring %d additional CS pins for multi-display\n", config.display_count - 1);
        for (uint8_t i = 0; i < config.display_count; i++) {
            gpio_init(config.cs_pins[i]);
            gpio_set_dir(config.cs_pins[i], GPIO_OUT);
            gpio_put(config.cs_pins[i], true); // Deselected
        }
    }
    
    hardware_initialized = true;
    printf("[SH110X] Hardware configuration complete\n");
    return true;
}

//============================================================================
// SPI COMMUNICATION
//============================================================================

void Adafruit_SH110X::spi_write_command(uint8_t cmd) {
    uint cs_pin = (config.display_count > 1 && config.cs_pins) ? 
                  config.cs_pins[current_display] : config.spi_cs_pin;
    
    gpio_put(config.spi_dc_pin, false);  // Command mode
    gpio_put(cs_pin, false);             // Select device
    spi_write_blocking(spi_instance, &cmd, 1);
    gpio_put(cs_pin, true);              // Deselect device
}

void Adafruit_SH110X::spi_write_data(uint8_t data) {
    uint cs_pin = (config.display_count > 1 && config.cs_pins) ? 
                  config.cs_pins[current_display] : config.spi_cs_pin;
    
    gpio_put(config.spi_dc_pin, true);   // Data mode
    gpio_put(cs_pin, false);             // Select device
    spi_write_blocking(spi_instance, &data, 1);
    gpio_put(cs_pin, true);              // Deselect device
}

void Adafruit_SH110X::spi_write_data_buffer(const uint8_t* data, size_t length) {
    uint cs_pin = (config.display_count > 1 && config.cs_pins) ? 
                  config.cs_pins[current_display] : config.spi_cs_pin;
    
    gpio_put(config.spi_dc_pin, true);   // Data mode
    gpio_put(cs_pin, false);             // Select device
    spi_write_blocking(spi_instance, data, length);
    gpio_put(cs_pin, true);              // Deselect device
}

//============================================================================
// DISPLAY OPERATIONS
//============================================================================

void Adafruit_SH110X::clearDisplay() {
    if (buffer) {
        memset(buffer, 0, buffer_size);
    }
}

void Adafruit_SH110X::invertDisplay(bool i) {
    spi_write_command(i ? SH110X_INVERTDISPLAY : SH110X_NORMALDISPLAY);
}

void Adafruit_SH110X::dim(bool dim) {
    // Set contrast based on dim setting
    spi_write_command(SH110X_SETCONTRAST);
    spi_write_command(dim ? 0x20 : 0xFF); // Low contrast when dim, high when not
}

void Adafruit_SH110X::display() {
    // Base implementation - this should be overridden by derived classes
    // For now, provide a simple implementation that does nothing
    // Each display type (SH1106, SH1107, etc.) has different addressing modes
}

//============================================================================
// GRAPHICS OVERRIDES
//============================================================================

void Adafruit_SH110X::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
        // Apply rotation
        switch (getRotation()) {
        case 1:
            std::swap(x, y);
            x = WIDTH - x - 1;
            break;
        case 2:
            x = WIDTH - x - 1;
            y = HEIGHT - y - 1;
            break;
        case 3:
            std::swap(x, y);
            y = HEIGHT - y - 1;
            break;
        }
        
        // Calculate buffer position
        uint16_t pos = x + (y / 8) * WIDTH;
        uint8_t bit = y & 7;
        
        if (buffer && pos < buffer_size) {
            switch (color) {
            case SH110X_WHITE:
                buffer[pos] |= (1 << bit);
                break;
            case SH110X_BLACK:
                buffer[pos] &= ~(1 << bit);
                break;
            case SH110X_INVERSE:
                buffer[pos] ^= (1 << bit);
                break;
            }
        }
    }
}

void Adafruit_SH110X::startWrite() {
    // Could add optimizations here for batch writes
}

void Adafruit_SH110X::endWrite() {
    // Could add optimizations here for batch writes
}

//============================================================================
// MULTI-DISPLAY SUPPORT
//============================================================================

void Adafruit_SH110X::selectDisplay(uint8_t display_index) {
    if (display_index < config.display_count) {
        current_display = display_index;
    }
}

//============================================================================
// MULTI-DISPLAY MANAGER IMPLEMENTATION
//============================================================================

SH110X_MultiDisplay::SH110X_MultiDisplay(Adafruit_SH110X** displays, uint8_t count)
    : displays(displays), display_count(count) {
}

void SH110X_MultiDisplay::clearAll() {
    for (uint8_t i = 0; i < display_count; i++) {
        if (displays[i]) {
            displays[i]->clearDisplay();
        }
    }
}

void SH110X_MultiDisplay::displayAll() {
    for (uint8_t i = 0; i < display_count; i++) {
        if (displays[i]) {
            displays[i]->display();
        }
    }
}

void SH110X_MultiDisplay::invertAll(bool invert) {
    for (uint8_t i = 0; i < display_count; i++) {
        if (displays[i]) {
            displays[i]->invertDisplay(invert);
        }
    }
}