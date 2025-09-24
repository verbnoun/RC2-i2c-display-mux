//============================================================================
// Adafruit SH1107 128x128 OLED Display Driver Implementation - Pico SDK Port
// Specific implementation for SH1107 128x128 pixel displays
// Based on working custom driver with DC-DC converter fix
//============================================================================

#include "Adafruit_SH1107.h"
#include "pico/stdlib.h"
#include <cstdio>

//============================================================================
// CONSTRUCTOR
//============================================================================

Adafruit_SH1107::Adafruit_SH1107(const SH1107_Config& config) 
    : Adafruit_SH110X(config) {
    
    // Set SH1107-specific parameters
    buffer_size = SH1107_BUFFER_SIZE;
    page_offset = 0;    // SH1107 doesn't need page offset
    column_offset = 0;  // SH1107 doesn't need column offset
}

//============================================================================
// DISPLAY INITIALIZATION
//============================================================================

bool Adafruit_SH1107::init_display() {
    printf("[SH1107] Starting display initialization...\n");
    
    // Hardware reset
    printf("[SH1107] Hardware reset: RST pin %d LOW\n", config.spi_rst_pin);
    gpio_put(config.spi_rst_pin, false);
    sleep_ms(10);
    printf("[SH1107] Hardware reset: RST pin %d HIGH\n", config.spi_rst_pin);
    gpio_put(config.spi_rst_pin, true);
    sleep_ms(10);
    
    // Send initialization sequence (based on working custom driver)
    printf("[SH1107] Sending SH1107 initialization commands...\n");
    send_init_sequence();
    printf("[SH1107] Display initialization completed\n");
    
    return true;
}

void Adafruit_SH1107::send_init_sequence() {
    // SH1107-specific initialization sequence (proven working from custom driver)
    
    // 1. Turn display off first
    spi_write_command(SH1107_DISPLAY_OFF);
    sleep_ms(10);
    
    // 2. Set display clock divide ratio/oscillator frequency  
    spi_write_command(SH1107_SET_DISPLAY_CLOCK);
    spi_write_command(0x51);  // Adafruit proven value
    sleep_ms(2);
    
    // 3. Set memory addressing mode (CRITICAL for SH1107)
    spi_write_command(SH1107_SET_MEMORY_MODE);
    spi_write_command(0x00);  // Page addressing mode
    sleep_ms(2);
    
    // 4. Set contrast (brightness)
    spi_write_command(SH1107_SET_CONTRAST);
    spi_write_command(0xFF);  // Maximum contrast for visibility
    sleep_ms(2);
    
    // 5. **CRITICAL: Enable DC-DC converter (SH1107-specific, NOT SSD1306!)**
    printf("[SH1107] CRITICAL: Enabling DC-DC converter (0xAD + 0x8A)...\n");
    spi_write_command(SH1107_SET_DCDC);
    spi_write_command(0x8A);  // Enable DC-DC converter
    sleep_ms(10);  // Give DC-DC time to stabilize
    printf("[SH1107] DC-DC converter enabled\n");
    
    // 6. Set segment re-mapping
    spi_write_command(SH1107_SEG_REMAP_NORMAL);  // Normal segment mapping
    sleep_ms(2);
    
    // 7. Set COM output scan direction
    spi_write_command(SH1107_COM_SCAN_INC);  // Normal scan direction
    sleep_ms(2);
    
    // 8. Set display start line
    spi_write_command(SH1107_SET_START_LINE | 0x00);  // Start line 0
    sleep_ms(2);
    
    // 9. Set display offset (important for 128x128)
    spi_write_command(SH1107_SET_DISPLAY_OFFSET);
    spi_write_command(0x60);  // Adafruit uses 0x60 for 128x128
    sleep_ms(2);
    
    // 10. Set pre-charge period
    spi_write_command(SH1107_SET_PRECHARGE);
    spi_write_command(0x22);  // Adafruit proven value
    sleep_ms(2);
    
    // 11. Set VCOM deselect level
    spi_write_command(SH1107_SET_VCOM_DETECT);
    spi_write_command(0x35);  // Adafruit proven value
    sleep_ms(2);
    
    // 12. Set multiplex ratio for 128x128 display
    spi_write_command(SH1107_SET_MULTIPLEX);
    spi_write_command(0x7F);  // 128 rows (0x7F = 127 + 1)
    sleep_ms(2);
    
    // 13. Set display to show RAM contents (not force all pixels on)
    spi_write_command(SH1107_DISPLAY_ALL_ON_RESUME);
    sleep_ms(2);
    
    // 14. Set normal display (not inverted)
    spi_write_command(SH1107_NORMAL_DISPLAY);
    sleep_ms(2);
    
    // 15. Turn display on
    printf("[SH1107] Turning display ON (0xAF command)...\n");
    spi_write_command(SH1107_DISPLAY_ON);
    sleep_ms(100);  // Give display time to turn on
    printf("[SH1107] Display should now be ON and ready\n");
}

//============================================================================
// DISPLAY UPDATE
//============================================================================

void Adafruit_SH1107::display() {
    if (!buffer) {
        printf("[SH1107] display() called but buffer is null!\n");
        return;
    }
    
    printf("[SH1107] === Starting display() - sending buffer to hardware ===\n");
    printf("[SH1107] Buffer size: %d bytes, sending %d pages\n", SH1107_BUFFER_SIZE, SH1107_PAGES);
    
    // Check first few bytes of buffer
    printf("[SH1107] Buffer content preview: [0]=%02X [1]=%02X [2]=%02X [3]=%02X\n", 
           buffer[0], buffer[1], buffer[2], buffer[3]);
    
    // SH1107 uses page-based addressing for 128x128
    // 16 pages (0-15), each page is 8 pixels high, 128 pixels wide
    for (uint8_t page = 0; page < SH1107_PAGES; page++) {
        printf("[SH1107] Sending page %d (address 0x%02X)...\n", page, SH1107_SET_PAGE_ADDR + page);
        
        // Set page address (B0-BF for pages 0-15)
        spi_write_command(SH1107_SET_PAGE_ADDR + page);
        
        // Set column address to start (0x00-0x0F for low nibble, 0x10-0x1F for high nibble)
        spi_write_command(SH1107_SET_LOW_COL | 0x00);   // Lower column start address
        spi_write_command(SH1107_SET_HIGH_COL | 0x00);  // Higher column start address
        
        // Send 128 bytes for this page (8 pixel rows)
        printf("[SH1107] Sending %d bytes for page %d\n", SH1107_PAGE_SIZE, page);
        spi_write_data_buffer(&buffer[page * SH1107_PAGE_SIZE], SH1107_PAGE_SIZE);
        
        // Add small delay to slow down transmission for observation
        sleep_ms(50);
        printf("[SH1107] Page %d sent\n", page);
    }
    
    printf("[SH1107] === Display buffer transmission complete ===\n");
}

//============================================================================
// SH1107-SPECIFIC FUNCTIONS
//============================================================================

void Adafruit_SH1107::setContrast(uint8_t contrast) {
    spi_write_command(SH1107_SET_CONTRAST);
    spi_write_command(contrast);
}

void Adafruit_SH1107::setDisplayOffset(uint8_t offset) {
    spi_write_command(SH1107_SET_DISPLAY_OFFSET);
    spi_write_command(offset);
}