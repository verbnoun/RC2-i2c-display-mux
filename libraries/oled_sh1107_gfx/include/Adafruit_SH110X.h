//============================================================================
// Adafruit SH110X OLED Display Base Class - Pico SDK Port
// Base class for SH110X family displays (SH1106, SH1107, etc.)
//============================================================================

#pragma once

#include "Adafruit_GFX.h"
#include "SH1107_Config.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

//============================================================================
// SH110X COMMAND DEFINITIONS
//============================================================================

#define SH110X_MEMORYMODE 0x20          ///< See datasheet
#define SH110X_COLUMNADDR 0x21          ///< See datasheet
#define SH110X_PAGEADDR 0x22            ///< See datasheet
#define SH110X_SETCONTRAST 0x81         ///< See datasheet
#define SH110X_CHARGEPUMP 0x8D          ///< See datasheet
#define SH110X_SEGREMAP 0xA0            ///< See datasheet
#define SH110X_DISPLAYALLON_RESUME 0xA4 ///< See datasheet
#define SH110X_DISPLAYALLON 0xA5        ///< Not currently used
#define SH110X_NORMALDISPLAY 0xA6       ///< See datasheet
#define SH110X_INVERTDISPLAY 0xA7       ///< See datasheet
#define SH110X_SETMULTIPLEX 0xA8        ///< See datasheet
#define SH110X_DCDC 0xAD                ///< See datasheet (SH1107 DC-DC converter)
#define SH110X_DISPLAYOFF 0xAE          ///< See datasheet
#define SH110X_DISPLAYON 0xAF           ///< See datasheet
#define SH110X_SETPAGEADDR 0xB0         ///< See datasheet
#define SH110X_COMSCANINC 0xC0          ///< Not currently used
#define SH110X_COMSCANDEC 0xC8          ///< See datasheet
#define SH110X_SETDISPLAYOFFSET 0xD3    ///< See datasheet
#define SH110X_SETDISPLAYCLOCKDIV 0xD5  ///< See datasheet
#define SH110X_SETPRECHARGE 0xD9        ///< See datasheet
#define SH110X_SETCOMPINS 0xDA          ///< See datasheet
#define SH110X_SETVCOMDETECT 0xDB       ///< See datasheet
#define SH110X_SETLOWCOLUMN 0x00        ///< Not currently used
#define SH110X_SETHIGHCOLUMN 0x10       ///< Not currently used
#define SH110X_SETSTARTLINE 0x40        ///< See datasheet

//============================================================================
// ADAFRUIT SH110X BASE CLASS
//============================================================================

class Adafruit_SH110X : public Adafruit_GFX {
public:
    // Constructor
    Adafruit_SH110X(const SH1107_Config& config);
    virtual ~Adafruit_SH110X();

    // Initialization
    bool begin();
    void clearDisplay();
    void invertDisplay(bool i);
    virtual void display();
    void dim(bool dim);

    // Override GFX functions for optimization
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void startWrite() override;
    void endWrite() override;

    // Buffer management
    uint8_t* getBuffer() { return buffer; }
    void setBuffer(uint8_t* buf) { buffer = buf; }

    // Multi-display support
    void selectDisplay(uint8_t display_index);
    uint8_t getCurrentDisplay() const { return current_display; }
    uint8_t getDisplayCount() const { return config.display_count; }

protected:
    const SH1107_Config& config;  ///< Display configuration
    uint8_t* buffer;              ///< Display buffer
    bool buffer_allocated;        ///< Whether buffer was allocated by this class
    uint8_t current_display;      ///< Currently selected display (for multi-display)

    // SPI communication functions
    void spi_write_command(uint8_t cmd);
    void spi_write_data(uint8_t data);
    void spi_write_data_buffer(const uint8_t* buffer, size_t length);

    // Hardware initialization  
    bool init_hardware();
    virtual bool init_display() = 0;  // Must be implemented by specific display

    // Display-specific parameters (set by derived classes)
    uint16_t buffer_size;
    uint8_t page_offset;
    uint8_t column_offset;

private:
    // SPI instance
    spi_inst_t* spi_instance;
    bool hardware_initialized;
};

//============================================================================
// MULTI-DISPLAY MANAGER
//============================================================================

class SH110X_MultiDisplay {
public:
    SH110X_MultiDisplay(Adafruit_SH110X** displays, uint8_t count);
    
    // Operations on all displays
    void clearAll();
    void displayAll();
    void invertAll(bool invert);
    
    // Individual display access
    Adafruit_SH110X* getDisplay(uint8_t index) { 
        return (index < display_count) ? displays[index] : nullptr; 
    }
    
    uint8_t getCount() const { return display_count; }

private:
    Adafruit_SH110X** displays;
    uint8_t display_count;
};