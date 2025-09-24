//============================================================================
// SH1107 OLED Display Configuration
// Modular configuration for GPIO pins and display parameters
//============================================================================

#pragma once

#include <stdint.h>
#include "pico/stdlib.h"

//============================================================================
// DISPLAY CONFIGURATION STRUCTURE
//============================================================================

struct SH1107_Config {
    // SPI Pin Configuration (must be set by application)
    uint spi_sck_pin;      // SPI Clock pin
    uint spi_mosi_pin;     // SPI Data Out pin  
    uint spi_dc_pin;       // Data/Command Select pin
    uint spi_rst_pin;      // Reset pin
    uint spi_cs_pin;       // Chip Select pin
    
    // SPI Configuration
    uint32_t spi_frequency;  // SPI frequency in Hz (default: 10MHz)
    uint spi_instance;       // SPI instance (0 or 1, default: 0)
    
    // Display Parameters
    uint16_t display_width;   // Display width in pixels (default: 128)
    uint16_t display_height;  // Display height in pixels (default: 128)
    uint8_t display_rotation; // Display rotation (0, 1, 2, 3)
    
    // Memory Management
    bool use_external_buffer; // Use external buffer vs internal allocation
    uint8_t* external_buffer; // Pointer to external buffer if used
    
    // Multi-Display Support
    uint8_t display_count;    // Number of displays (1-8, for CS multiplexing)
    uint* cs_pins;           // Array of CS pins for multiple displays
    
    // Constructor with defaults
    SH1107_Config() :
        spi_sck_pin(0),        // Must be set by application
        spi_mosi_pin(0),       // Must be set by application  
        spi_dc_pin(0),         // Must be set by application
        spi_rst_pin(0),        // Must be set by application
        spi_cs_pin(0),         // Must be set by application
        spi_frequency(10000000), // 10MHz
        spi_instance(0),       // spi0
        display_width(128),
        display_height(128),
        display_rotation(0),
        use_external_buffer(false),
        external_buffer(nullptr),
        display_count(1),
        cs_pins(nullptr)
    {}
};

//============================================================================
// CONFIGURATION VALIDATION
//============================================================================

// Validate configuration before use
inline bool validate_config(const SH1107_Config& config) {
    // Check required pins are set
    if (config.spi_sck_pin == 0 || config.spi_mosi_pin == 0 || 
        config.spi_dc_pin == 0 || config.spi_rst_pin == 0 || 
        config.spi_cs_pin == 0) {
        return false;
    }
    
    // Check SPI instance
    if (config.spi_instance > 1) return false;
    
    // Check display parameters
    if (config.display_width == 0 || config.display_height == 0) return false;
    if (config.display_rotation > 3) return false;
    
    // Check multi-display config
    if (config.display_count > 8) return false;
    if (config.display_count > 1 && config.cs_pins == nullptr) return false;
    
    return true;
}

//============================================================================
// COMMON CONFIGURATIONS
//============================================================================

// Pre-defined configurations for common setups
namespace SH1107_Configs {
    
    // Single display configuration template
    inline SH1107_Config single_display(uint sck, uint mosi, uint dc, uint rst, uint cs) {
        SH1107_Config config;
        config.spi_sck_pin = sck;
        config.spi_mosi_pin = mosi;
        config.spi_dc_pin = dc;
        config.spi_rst_pin = rst;
        config.spi_cs_pin = cs;
        return config;
    }
    
    // MIDI controller 5-display template
    inline SH1107_Config midi_controller_5x(uint sck, uint mosi, uint dc, uint rst, 
                                           uint cs1, uint cs2, uint cs3, uint cs4, uint cs5) {
        SH1107_Config config;
        config.spi_sck_pin = sck;
        config.spi_mosi_pin = mosi;
        config.spi_dc_pin = dc;
        config.spi_rst_pin = rst;
        config.display_count = 5;
        
        // Allocate CS pins array
        static uint cs_pins[5] = {cs1, cs2, cs3, cs4, cs5};
        config.cs_pins = cs_pins;
        config.spi_cs_pin = cs1; // Primary CS pin
        
        return config;
    }
}