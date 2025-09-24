//============================================================================
// Adafruit SH1107 128x128 OLED Display Driver - Pico SDK Port
// Specific implementation for SH1107 128x128 pixel displays
//============================================================================

#pragma once

#include "Adafruit_SH110X.h"

//============================================================================
// SH1107-SPECIFIC COMMANDS
//============================================================================

#define SH1107_SET_LOW_COL          0x00  ///< Set lower column address (0x00-0x0F)
#define SH1107_SET_HIGH_COL         0x10  ///< Set higher column address (0x10-0x1F)
#define SH1107_SET_PAGE_ADDR        0xB0  ///< Set page address (0xB0-0xBF)
#define SH1107_SET_DISPLAY_OFFSET   0xD3  ///< Set display offset
#define SH1107_SET_DCDC             0xAD  ///< DC-DC converter control
#define SH1107_SET_DISPLAY_CLOCK    0xD5  ///< Set display clock divide ratio
#define SH1107_SET_PRECHARGE        0xD9  ///< Set pre-charge period
#define SH1107_SET_VCOM_DETECT      0xDB  ///< Set VCOM deselect level
#define SH1107_SET_MEMORY_MODE      0x20  ///< Memory addressing mode
#define SH1107_SET_CONTRAST         0x81  ///< Set contrast
#define SH1107_SET_MULTIPLEX        0xA8  ///< Set multiplex ratio
#define SH1107_DISPLAY_OFF          0xAE  ///< Display off
#define SH1107_DISPLAY_ON           0xAF  ///< Display on
#define SH1107_NORMAL_DISPLAY       0xA6  ///< Normal display (not inverted)
#define SH1107_INVERT_DISPLAY       0xA7  ///< Invert display
#define SH1107_DISPLAY_ALL_ON_RESUME 0xA4 ///< Resume to RAM content display
#define SH1107_SET_START_LINE       0x40  ///< Set display start line (0x40-0x7F)
#define SH1107_SEG_REMAP_NORMAL     0xA0  ///< Normal segment mapping
#define SH1107_SEG_REMAP_REVERSE    0xA1  ///< Reverse segment mapping
#define SH1107_COM_SCAN_INC         0xC0  ///< Normal COM scan direction
#define SH1107_COM_SCAN_DEC         0xC8  ///< Reverse COM scan direction

//============================================================================
// ADAFRUIT SH1107 CLASS
//============================================================================

class Adafruit_SH1107 : public Adafruit_SH110X {
public:
    // Constructor for 128x128 display
    Adafruit_SH1107(const SH1107_Config& config);

    // SH1107-specific functions
    void setContrast(uint8_t contrast);
    void setDisplayOffset(uint8_t offset);
    
    // Override display function for SH1107 page addressing
    void display() override;

protected:
    // Override base class initialization
    bool init_display() override;

private:
    // SH1107-specific initialization sequence
    void send_init_sequence();
    
    // Display parameters for 128x128 SH1107
    static const uint16_t SH1107_WIDTH = 128;
    static const uint16_t SH1107_HEIGHT = 128;
    static const uint8_t SH1107_PAGES = 16;    // 128/8 = 16 pages
    static const uint8_t SH1107_PAGE_SIZE = 128; // 128 bytes per page
    static const uint16_t SH1107_BUFFER_SIZE = SH1107_PAGES * SH1107_PAGE_SIZE; // 2048 bytes
};

//============================================================================
// CONVENIENCE FUNCTIONS
//============================================================================

namespace SH1107 {
    // Factory function for single display
    inline Adafruit_SH1107* createSingleDisplay(uint sck, uint mosi, uint dc, uint rst, uint cs) {
        SH1107_Config config = SH1107_Configs::single_display(sck, mosi, dc, rst, cs);
        return new Adafruit_SH1107(config);
    }
    
    // Factory function for MIDI controller setup (5 displays)
    inline Adafruit_SH1107** createMIDIController(uint sck, uint mosi, uint dc, uint rst,
                                                 uint cs1, uint cs2, uint cs3, uint cs4, uint cs5) {
        // Create individual configs for each display
        Adafruit_SH1107** displays = new Adafruit_SH1107*[5];
        
        displays[0] = new Adafruit_SH1107(SH1107_Configs::single_display(sck, mosi, dc, rst, cs1));
        displays[1] = new Adafruit_SH1107(SH1107_Configs::single_display(sck, mosi, dc, rst, cs2));
        displays[2] = new Adafruit_SH1107(SH1107_Configs::single_display(sck, mosi, dc, rst, cs3));
        displays[3] = new Adafruit_SH1107(SH1107_Configs::single_display(sck, mosi, dc, rst, cs4));
        displays[4] = new Adafruit_SH1107(SH1107_Configs::single_display(sck, mosi, dc, rst, cs5));
        
        return displays;
    }
}