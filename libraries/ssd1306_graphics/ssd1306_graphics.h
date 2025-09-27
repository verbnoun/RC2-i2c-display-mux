//============================================================================
// SSD1306 Graphics Library for Raspberry Pi Pico
// High-level graphics and text rendering for SSD1306 OLED displays
//============================================================================

#ifndef SSD1306_GRAPHICS_H
#define SSD1306_GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

//============================================================================
// DISPLAY CONFIGURATION
//============================================================================
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64
#define SSD1306_PAGES           8     // 64 / 8 = 8 pages
#define SSD1306_BUFFER_SIZE     (SSD1306_WIDTH * SSD1306_PAGES)
#define SSD1306_I2C_ADDRESS     0x3C

//============================================================================
// SSD1306 COMMANDS
//============================================================================
#define SSD1306_DISPLAY_OFF          0xAE
#define SSD1306_DISPLAY_ON           0xAF
#define SSD1306_DISPLAY_ALL_ON_RESUME 0xA4
#define SSD1306_DISPLAY_ALL_ON       0xA5
#define SSD1306_DISPLAY_NORMAL       0xA6
#define SSD1306_DISPLAY_INVERT       0xA7
#define SSD1306_SET_CONTRAST         0x81
#define SSD1306_CHARGE_PUMP          0x8D
#define SSD1306_SET_MEMORY_MODE      0x20
#define SSD1306_SET_COLUMN_ADDR      0x21
#define SSD1306_SET_PAGE_ADDR        0x22
#define SSD1306_SET_COM_SCAN_INC     0xC0
#define SSD1306_SET_COM_SCAN_DEC     0xC8
#define SSD1306_SET_SEG_REMAP        0xA0
#define SSD1306_SET_COM_PINS         0xDA
#define SSD1306_SET_DISP_CLOCK_DIV   0xD5
#define SSD1306_SET_PRECHARGE        0xD9
#define SSD1306_SET_VCOM_DESEL       0xDB
#define SSD1306_SET_START_LINE       0x40
#define SSD1306_SET_DISP_OFFSET      0xD3

//============================================================================
// FONT STRUCTURE
//============================================================================
typedef struct {
    uint8_t width;           // Character width in pixels
    uint8_t height;          // Character height in pixels
    uint8_t first_char;      // ASCII code of first character
    uint8_t last_char;       // ASCII code of last character
    const uint8_t (*font_data)[16]; // 2D Font bitmap data array [char][row]
} ssd1306_font_t;

//============================================================================
// GRAPHICS CONTEXT
//============================================================================
typedef struct {
    uint8_t buffer[SSD1306_BUFFER_SIZE];  // Display buffer
    i2c_inst_t* i2c_port;                 // I2C instance
    bool initialized;                      // Initialization status
    const ssd1306_font_t* current_font;   // Current font
} ssd1306_t;

//============================================================================
// CORE FUNCTIONS
//============================================================================

/**
 * Initialize SSD1306 display
 * @param display Pointer to SSD1306 context
 * @param i2c_port I2C port to use (i2c0 or i2c1)
 * @return true if successful, false otherwise
 */
bool ssd1306_init(ssd1306_t* display, i2c_inst_t* i2c_port);

/**
 * Send command to SSD1306
 * @param display Pointer to SSD1306 context
 * @param cmd Command byte to send
 * @return true if successful, false otherwise
 */
bool ssd1306_send_command(ssd1306_t* display, uint8_t cmd);

/**
 * Update entire display from buffer
 * @param display Pointer to SSD1306 context
 * @return true if successful, false otherwise
 */
bool ssd1306_display(ssd1306_t* display);

/**
 * Clear display buffer (set all pixels to black)
 * @param display Pointer to SSD1306 context
 */
void ssd1306_clear(ssd1306_t* display);

/**
 * Fill display buffer (set all pixels to white)
 * @param display Pointer to SSD1306 context
 */
void ssd1306_fill(ssd1306_t* display);

//============================================================================
// PIXEL AND DRAWING FUNCTIONS
//============================================================================

/**
 * Set a single pixel in the buffer
 * @param display Pointer to SSD1306 context
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-63)
 * @param color 1 for white, 0 for black
 */
void ssd1306_set_pixel(ssd1306_t* display, uint8_t x, uint8_t y, bool color);

/**
 * Get a single pixel from the buffer
 * @param display Pointer to SSD1306 context
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-63)
 * @return true if pixel is white, false if black
 */
bool ssd1306_get_pixel(ssd1306_t* display, uint8_t x, uint8_t y);

/**
 * Draw a horizontal line
 * @param display Pointer to SSD1306 context
 * @param x X starting coordinate
 * @param y Y coordinate
 * @param width Line width
 * @param color 1 for white, 0 for black
 */
void ssd1306_draw_hline(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t width, bool color);

/**
 * Draw a vertical line
 * @param display Pointer to SSD1306 context
 * @param x X coordinate
 * @param y Y starting coordinate
 * @param height Line height
 * @param color 1 for white, 0 for black
 */
void ssd1306_draw_vline(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t height, bool color);

/**
 * Draw a rectangle
 * @param display Pointer to SSD1306 context
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param width Rectangle width
 * @param height Rectangle height
 * @param color 1 for white, 0 for black
 * @param filled true to fill rectangle, false for outline only
 */
void ssd1306_draw_rect(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool color, bool filled);

//============================================================================
// TEXT AND FONT FUNCTIONS
//============================================================================

/**
 * Set the current font
 * @param display Pointer to SSD1306 context
 * @param font Pointer to font structure
 */
void ssd1306_set_font(ssd1306_t* display, const ssd1306_font_t* font);

/**
 * Draw a single character
 * @param display Pointer to SSD1306 context
 * @param x X coordinate
 * @param y Y coordinate
 * @param ch Character to draw
 * @param color 1 for white, 0 for black
 * @return Width of the character drawn
 */
uint8_t ssd1306_draw_char(ssd1306_t* display, uint8_t x, uint8_t y, char ch, bool color);

/**
 * Draw a string
 * @param display Pointer to SSD1306 context
 * @param x X starting coordinate
 * @param y Y coordinate
 * @param str String to draw
 * @param color 1 for white, 0 for black
 * @return Total width of the string drawn
 */
uint16_t ssd1306_draw_string(ssd1306_t* display, uint8_t x, uint8_t y, const char* str, bool color);

/**
 * Calculate the width of a string in pixels
 * @param display Pointer to SSD1306 context
 * @param str String to measure
 * @return Width in pixels
 */
uint16_t ssd1306_string_width(ssd1306_t* display, const char* str);

//============================================================================
// UTILITY FUNCTIONS
//============================================================================

/**
 * Set contrast (brightness)
 * @param display Pointer to SSD1306 context
 * @param contrast Contrast value (0-255)
 * @return true if successful, false otherwise
 */
bool ssd1306_set_contrast(ssd1306_t* display, uint8_t contrast);

/**
 * Turn display on
 * @param display Pointer to SSD1306 context
 * @return true if successful, false otherwise
 */
bool ssd1306_display_on(ssd1306_t* display);

/**
 * Turn display off
 * @param display Pointer to SSD1306 context
 * @return true if successful, false otherwise
 */
bool ssd1306_display_off(ssd1306_t* display);

/**
 * Invert display colors
 * @param display Pointer to SSD1306 context
 * @param invert true to invert, false for normal
 * @return true if successful, false otherwise
 */
bool ssd1306_invert_display(ssd1306_t* display, bool invert);

//============================================================================
// EXTERNAL FONTS
// Include font headers as needed, e.g.:
// #include "fonts/font8x16.h"
//============================================================================

#ifdef __cplusplus
}
#endif

#endif // SSD1306_GRAPHICS_H