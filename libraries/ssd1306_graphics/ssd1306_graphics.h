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
    const void *font_data;   // 2D Font bitmap data array [char][row] - cast to appropriate type in code
} ssd1306_font_t;

//============================================================================
// GRAPHICS CONTEXT
//============================================================================
typedef struct {
    uint8_t buffer[SSD1306_BUFFER_SIZE];  // Display buffer
    i2c_inst_t* i2c_port;                 // I2C instance
    bool initialized;                      // Initialization status
    const ssd1306_font_t* current_font;   // Current font
    
    // Partial update support
    uint8_t dirty_x_min, dirty_y_min;     // Top-left of dirty region
    uint8_t dirty_x_max, dirty_y_max;     // Bottom-right of dirty region  
    bool has_dirty_region;                 // True if any region is dirty
    uint8_t update_priority;               // Display update priority (0=highest)
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

/**
 * Clear a specific region of the display buffer
 * @param display Pointer to SSD1306 context
 * @param x X coordinate of region
 * @param y Y coordinate of region
 * @param width Width of region
 * @param height Height of region
 */
void ssd1306_clear_region(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t width, uint8_t height);

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

/**
 * Draw a circle
 * @param display Pointer to SSD1306 context
 * @param center_x X coordinate of circle center
 * @param center_y Y coordinate of circle center
 * @param radius Circle radius in pixels
 * @param color 1 for white, 0 for black
 * @param filled true to fill circle, false for outline only
 */
void ssd1306_draw_circle(ssd1306_t* display, uint8_t center_x, uint8_t center_y, uint8_t radius, bool color, bool filled);

/**
 * Draw a 5-point star
 * @param display Pointer to SSD1306 context
 * @param center_x X coordinate of star center
 * @param center_y Y coordinate of star center
 * @param outer_radius Radius to outer points of star
 * @param color 1 for white, 0 for black
 * @param filled true to fill star, false for outline only
 */
void ssd1306_draw_star(ssd1306_t* display, uint8_t center_x, uint8_t center_y, uint8_t outer_radius, bool color, bool filled);

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
// ANIMATION AND TIMING FUNCTIONS
//============================================================================

/**
 * Set target frame rate for animations
 * @param fps Frames per second (1-60)
 */
void ssd1306_set_fps(uint8_t fps);

/**
 * Get current frame rate setting
 * @return Current FPS setting
 */
uint8_t ssd1306_get_fps(void);

/**
 * Wait for the appropriate frame delay based on current FPS setting
 */
void ssd1306_frame_delay(void);

/**
 * Get frame delay in milliseconds for current FPS
 * @return Delay in milliseconds
 */
uint32_t ssd1306_get_frame_delay_ms(void);

//============================================================================
// PARTIAL DISPLAY UPDATE FUNCTIONS
//============================================================================

/**
 * Update only a specific region of the display
 * @param display Pointer to SSD1306 context
 * @param x Starting X coordinate (0-127)
 * @param y Starting Y coordinate (0-63) 
 * @param width Width of region to update
 * @param height Height of region to update
 * @return true if successful, false otherwise
 */
bool ssd1306_display_partial(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t width, uint8_t height);

/**
 * Mark a region as dirty for next partial update
 * @param display Pointer to SSD1306 context
 * @param x Starting X coordinate
 * @param y Starting Y coordinate  
 * @param width Width of dirty region
 * @param height Height of dirty region
 */
void ssd1306_mark_dirty(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t width, uint8_t height);

/**
 * Update all dirty regions and clear dirty flags
 * @param display Pointer to SSD1306 context
 * @return true if any updates were performed
 */
bool ssd1306_display_dirty_regions(ssd1306_t* display);

/**
 * Set display update priority (lower number = higher priority)
 * @param display Pointer to SSD1306 context
 * @param priority Priority level (0=highest, 255=lowest)
 */
void ssd1306_set_priority(ssd1306_t* display, uint8_t priority);

/**
 * Get information about the dirty region for debugging
 * @param display Pointer to SSD1306 context
 * @param width Pointer to store dirty region width
 * @param height Pointer to store dirty region height  
 * @param pixels Pointer to store total dirty pixels
 * @return true if dirty region exists, false otherwise
 */
bool ssd1306_get_dirty_info(ssd1306_t* display, uint8_t* width, uint8_t* height, uint16_t* pixels);

//============================================================================
// PROGRESS BAR AND INDICATOR FUNCTIONS
//============================================================================

/**
 * Bar style options
 */
typedef enum {
    BAR_STYLE_FILLED,      // Solid filled bar
    BAR_STYLE_OUTLINED,    // Outlined bar only
    BAR_STYLE_SEGMENTED    // Segmented bar with gaps
} ssd1306_bar_style_t;

/**
 * Draw a progress bar
 * @param display Pointer to SSD1306 context
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param width Bar width in pixels
 * @param height Bar height in pixels
 * @param progress Current progress value
 * @param max_progress Maximum progress value
 */
void ssd1306_draw_progress_bar(ssd1306_t* display, uint8_t x, uint8_t y, 
                               uint8_t width, uint8_t height, 
                               uint8_t progress, uint8_t max_progress);

/**
 * Draw an indicator bar (value within range)
 * @param display Pointer to SSD1306 context
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param width Bar width in pixels
 * @param height Bar height in pixels
 * @param value Current value
 * @param min_val Minimum value
 * @param max_val Maximum value
 */
void ssd1306_draw_indicator_bar(ssd1306_t* display, uint8_t x, uint8_t y,
                                uint8_t width, uint8_t height,
                                uint8_t value, uint8_t min_val, uint8_t max_val);

/**
 * Draw a styled bar with different visual options
 * @param display Pointer to SSD1306 context
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param width Bar width in pixels
 * @param height Bar height in pixels
 * @param value Current value
 * @param max_value Maximum value
 * @param style Bar visual style
 */
void ssd1306_draw_styled_bar(ssd1306_t* display, uint8_t x, uint8_t y,
                             uint8_t width, uint8_t height,
                             uint8_t value, uint8_t max_value,
                             ssd1306_bar_style_t style);

//============================================================================
// JAPANESE FONTS AND CHARACTER MAPPING
//============================================================================

// Include Japanese character mapping system
#include "japanese_char_map.h"

// Include Japanese font headers
#include "fonts/misaki_8px_japanese.h"
#include "fonts/pixelmplus_10px_japanese.h"
#include "fonts/pixelmplus_12px_japanese.h"

//============================================================================
// JAPANESE TEXT FUNCTIONS
//============================================================================

/**
 * Draw a UTF-8 character (supports Japanese)
 * @param display Pointer to SSD1306 context
 * @param x X coordinate
 * @param y Y coordinate
 * @param utf8_char Pointer to UTF-8 character bytes
 * @param color 1 for white, 0 for black
 * @return Width of the character drawn
 */
uint8_t ssd1306_draw_utf8_char(ssd1306_t* display, uint8_t x, uint8_t y, const uint8_t* utf8_char, bool color);

/**
 * Draw a UTF-8 string (supports Japanese)
 * @param display Pointer to SSD1306 context
 * @param x X starting coordinate
 * @param y Y coordinate
 * @param utf8_str UTF-8 string to draw
 * @param color 1 for white, 0 for black
 * @return Total width of the string drawn
 */
uint16_t ssd1306_draw_utf8_string(ssd1306_t* display, uint8_t x, uint8_t y, const char* utf8_str, bool color);

/**
 * Calculate the width of a UTF-8 string in pixels (supports Japanese)
 * @param display Pointer to SSD1306 context
 * @param utf8_str UTF-8 string to measure
 * @return Width in pixels
 */
uint16_t ssd1306_utf8_string_width(ssd1306_t* display, const char* utf8_str);

//============================================================================
// INVERSION AND XOR FUNCTIONS
//============================================================================

/**
 * Set a pixel with XOR operation (toggle pixel state)
 * @param display Pointer to SSD1306 context
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-63)
 */
void ssd1306_xor_pixel(ssd1306_t* display, uint8_t x, uint8_t y);

/**
 * Invert all pixels in a rectangular region
 * @param display Pointer to SSD1306 context
 * @param x X coordinate of region
 * @param y Y coordinate of region
 * @param width Width of region to invert
 * @param height Height of region to invert
 */
void ssd1306_invert_region(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t width, uint8_t height);

/**
 * Draw a UTF-8 character with XOR operation (adapts to background)
 * @param display Pointer to SSD1306 context
 * @param x X coordinate
 * @param y Y coordinate
 * @param utf8_char Pointer to UTF-8 character bytes
 * @param color 1 for XOR with white pattern, 0 for XOR with black pattern
 * @return Width of the character drawn
 */
uint8_t ssd1306_draw_utf8_char_xor(ssd1306_t* display, uint8_t x, uint8_t y, const uint8_t* utf8_char, bool color);

/**
 * Draw a UTF-8 string with inverted pixels (XOR mode)
 * @param display Pointer to SSD1306 context
 * @param x X starting coordinate
 * @param y Y coordinate
 * @param utf8_str UTF-8 string to draw
 * @return Total width of the string drawn
 */
uint16_t ssd1306_draw_utf8_string_inverted(ssd1306_t* display, uint8_t x, uint8_t y, const char* utf8_str);

//============================================================================
// TEXT ANIMATION FUNCTIONS (Build Effects)
//============================================================================

/**
 * Draw a UTF-8 string with typewriter effect (progressive character reveal)
 * @param display Pointer to SSD1306 context
 * @param x X starting coordinate
 * @param y Y coordinate
 * @param utf8_str UTF-8 string to draw
 * @param progress_ms Current animation time in milliseconds
 * @param char_interval_ms Milliseconds per character
 * @param show_cursor true to show blinking cursor
 * @return Total width that would be drawn when complete
 */
uint16_t ssd1306_draw_utf8_string_typewriter(ssd1306_t* display, uint8_t x, uint8_t y, 
                                             const char* utf8_str, uint32_t progress_ms, 
                                             uint32_t char_interval_ms, bool show_cursor);

/**
 * Count actual UTF-8 characters in a string (not bytes)
 * @param utf8_str UTF-8 string to count
 * @return Number of UTF-8 characters
 */
uint16_t ssd1306_utf8_char_count(const char* utf8_str);

/**
 * Extract a substring with a specific number of UTF-8 characters
 * @param utf8_str Source UTF-8 string
 * @param char_count Number of characters to extract
 * @param buffer Output buffer for substring
 * @param buffer_size Size of output buffer
 * @return Number of bytes written to buffer
 */
uint16_t ssd1306_utf8_substring(const char* utf8_str, uint16_t char_count, char* buffer, uint16_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif // SSD1306_GRAPHICS_H