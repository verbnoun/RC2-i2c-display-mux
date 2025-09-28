//============================================================================
// SSD1306 Graphics Library Implementation
// High-level graphics and text rendering for SSD1306 OLED displays
//============================================================================

#include "ssd1306_graphics.h"
#include "japanese_char_map.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "pico/time.h"

//============================================================================
// CORE FUNCTIONS
//============================================================================

bool ssd1306_init(ssd1306_t* display, i2c_inst_t* i2c_port) {
    if (!display || !i2c_port) return false;
    
    display->i2c_port = i2c_port;
    display->initialized = false;
    display->current_font = NULL;  // No default font - must be set externally
    
    // Initialize partial update fields
    display->has_dirty_region = false;
    display->dirty_x_min = 0;
    display->dirty_y_min = 0;
    display->dirty_x_max = 0;
    display->dirty_y_max = 0;
    display->update_priority = 128;  // Default medium priority
    
    
    // Clear buffer
    memset(display->buffer, 0, SSD1306_BUFFER_SIZE);
    
    // Initialization sequence
    const uint8_t init_sequence[] = {
        SSD1306_DISPLAY_OFF,                    // Display off
        SSD1306_SET_MEMORY_MODE,       0x00,    // Horizontal addressing mode
        SSD1306_SET_START_LINE,                 // Set start line to 0
        SSD1306_SET_SEG_REMAP | 0x01,          // Column address 127 mapped to SEG0
        SSD1306_SET_COM_SCAN_DEC,              // Remapped mode, scan from COM[N-1] to COM0
        SSD1306_SET_COM_PINS,          0x12,    // COM pins hardware configuration
        SSD1306_SET_CONTRAST,          0x7F,    // Set contrast (0-255)
        SSD1306_SET_PRECHARGE,         0xF1,    // Set pre-charge period
        SSD1306_SET_VCOM_DESEL,        0x40,    // Set VCOMH deselect level
        SSD1306_DISPLAY_ALL_ON_RESUME,         // Resume to RAM content display
        SSD1306_DISPLAY_NORMAL,                // Normal display (not inverted)
        SSD1306_CHARGE_PUMP,           0x14,    // Charge pump setting: enable
        SSD1306_DISPLAY_ON                     // Display on
    };
    
    // Send initialization commands
    for (int i = 0; i < sizeof(init_sequence); i++) {
        if (!ssd1306_send_command(display, init_sequence[i])) {
            return false;
        }
    }
    
    display->initialized = true;
    return true;
}

bool ssd1306_send_command(ssd1306_t* display, uint8_t cmd) {
    if (!display || !display->i2c_port) return false;
    
    uint8_t buffer[2] = {0x00, cmd};  // Control byte (0x00) + command
    int result = i2c_write_timeout_us(display->i2c_port, SSD1306_I2C_ADDRESS, buffer, 2, false, 50000);
    return (result == 2);
}

bool ssd1306_display(ssd1306_t* display) {
    if (!display || !display->initialized) return false;
    
    // Set column address range (0 to 127)
    if (!ssd1306_send_command(display, SSD1306_SET_COLUMN_ADDR) ||
        !ssd1306_send_command(display, 0) ||
        !ssd1306_send_command(display, SSD1306_WIDTH - 1)) {
        return false;
    }
    
    // Set page address range (0 to 7)
    if (!ssd1306_send_command(display, SSD1306_SET_PAGE_ADDR) ||
        !ssd1306_send_command(display, 0) ||
        !ssd1306_send_command(display, SSD1306_PAGES - 1)) {
        return false;
    }
    
    // Send display data in chunks
    const int chunk_size = 128;  // Send 128 bytes at a time
    uint8_t data_buffer[chunk_size + 1];
    data_buffer[0] = 0x40;  // Data control byte
    
    for (int i = 0; i < SSD1306_BUFFER_SIZE; i += chunk_size) {
        int bytes_to_send = (i + chunk_size > SSD1306_BUFFER_SIZE) ? 
                           (SSD1306_BUFFER_SIZE - i) : chunk_size;
        
        memcpy(&data_buffer[1], &display->buffer[i], bytes_to_send);
        
        int result = i2c_write_timeout_us(display->i2c_port, SSD1306_I2C_ADDRESS, 
                                         data_buffer, bytes_to_send + 1, false, 100000);
        if (result != bytes_to_send + 1) {
            return false;
        }
    }
    
    return true;
}

void ssd1306_clear(ssd1306_t* display) {
    if (!display) return;
    memset(display->buffer, 0, SSD1306_BUFFER_SIZE);
    // Auto-mark entire screen as dirty
    ssd1306_mark_dirty(display, 0, 0, SSD1306_WIDTH, SSD1306_HEIGHT);
}

void ssd1306_fill(ssd1306_t* display) {
    if (!display) return;
    memset(display->buffer, 0xFF, SSD1306_BUFFER_SIZE);
    // Auto-mark entire screen as dirty
    ssd1306_mark_dirty(display, 0, 0, SSD1306_WIDTH, SSD1306_HEIGHT);
}

void ssd1306_clear_region(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    if (!display) return;
    
    // Use filled rectangle to clear region efficiently
    ssd1306_draw_rect(display, x, y, width, height, false, true);
    
    // Note: draw_rect already auto-marks the region as dirty
}

//============================================================================
// PIXEL AND DRAWING FUNCTIONS
//============================================================================

void ssd1306_set_pixel(ssd1306_t* display, uint8_t x, uint8_t y, bool color) {
    if (!display || x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    
    uint16_t index = x + (y / 8) * SSD1306_WIDTH;
    uint8_t bit = y % 8;
    
    if (color) {
        display->buffer[index] |= (1 << bit);
    } else {
        display->buffer[index] &= ~(1 << bit);
    }
}

bool ssd1306_get_pixel(ssd1306_t* display, uint8_t x, uint8_t y) {
    if (!display || x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return false;
    
    uint16_t index = x + (y / 8) * SSD1306_WIDTH;
    uint8_t bit = y % 8;
    
    return (display->buffer[index] & (1 << bit)) != 0;
}

void ssd1306_draw_hline(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t width, bool color) {
    if (!display) return;
    
    for (uint8_t i = 0; i < width && (x + i) < SSD1306_WIDTH; i++) {
        ssd1306_set_pixel(display, x + i, y, color);
    }
    
    // Auto-mark line region as dirty
    uint8_t actual_width = (x + width > SSD1306_WIDTH) ? (SSD1306_WIDTH - x) : width;
    ssd1306_mark_dirty(display, x, y, actual_width, 1);
}

void ssd1306_draw_vline(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t height, bool color) {
    if (!display) return;
    
    for (uint8_t i = 0; i < height && (y + i) < SSD1306_HEIGHT; i++) {
        ssd1306_set_pixel(display, x, y + i, color);
    }
    
    // Auto-mark line region as dirty
    uint8_t actual_height = (y + height > SSD1306_HEIGHT) ? (SSD1306_HEIGHT - y) : height;
    ssd1306_mark_dirty(display, x, y, 1, actual_height);
}

void ssd1306_draw_rect(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool color, bool filled) {
    if (!display) return;
    
    if (filled) {
        for (uint8_t j = 0; j < height && (y + j) < SSD1306_HEIGHT; j++) {
            // Note: hline will auto-mark its own dirty region
            ssd1306_draw_hline(display, x, y + j, width, color);
        }
    } else {
        // Draw outline (each line will auto-mark its own dirty region)
        ssd1306_draw_hline(display, x, y, width, color);                    // Top
        ssd1306_draw_hline(display, x, y + height - 1, width, color);      // Bottom
        ssd1306_draw_vline(display, x, y, height, color);                  // Left
        ssd1306_draw_vline(display, x + width - 1, y, height, color);      // Right
    }
    
    // Note: Individual lines auto-mark dirty, but for efficiency mark entire rect
    uint8_t actual_width = (x + width > SSD1306_WIDTH) ? (SSD1306_WIDTH - x) : width;
    uint8_t actual_height = (y + height > SSD1306_HEIGHT) ? (SSD1306_HEIGHT - y) : height;
    ssd1306_mark_dirty(display, x, y, actual_width, actual_height);
}

void ssd1306_draw_circle(ssd1306_t* display, uint8_t center_x, uint8_t center_y, uint8_t radius, bool color, bool filled) {
    if (!display || radius == 0) return;
    
    if (filled) {
        // Draw filled circle using horizontal lines
        for (int y = -radius; y <= radius; y++) {
            int x_width = (int)(sqrt(radius * radius - y * y) + 0.5);
            for (int x = -x_width; x <= x_width; x++) {
                ssd1306_set_pixel(display, center_x + x, center_y + y, color);
            }
        }
    } else {
        // Draw circle outline using Bresenham's circle algorithm
        int x = 0;
        int y = radius;
        int d = 3 - 2 * radius;
        
        while (y >= x) {
            // Draw 8 points symmetrically
            ssd1306_set_pixel(display, center_x + x, center_y + y, color);
            ssd1306_set_pixel(display, center_x - x, center_y + y, color);
            ssd1306_set_pixel(display, center_x + x, center_y - y, color);
            ssd1306_set_pixel(display, center_x - x, center_y - y, color);
            ssd1306_set_pixel(display, center_x + y, center_y + x, color);
            ssd1306_set_pixel(display, center_x - y, center_y + x, color);
            ssd1306_set_pixel(display, center_x + y, center_y - x, color);
            ssd1306_set_pixel(display, center_x - y, center_y - x, color);
            
            x++;
            if (d > 0) {
                y--;
                d = d + 4 * (x - y) + 10;
            } else {
                d = d + 4 * x + 6;
            }
        }
    }
    
    // Auto-mark bounding box as dirty
    uint8_t x_min = (center_x > radius) ? center_x - radius : 0;
    uint8_t y_min = (center_y > radius) ? center_y - radius : 0;
    uint8_t x_max = (center_x + radius < SSD1306_WIDTH) ? center_x + radius : SSD1306_WIDTH - 1;
    uint8_t y_max = (center_y + radius < SSD1306_HEIGHT) ? center_y + radius : SSD1306_HEIGHT - 1;
    ssd1306_mark_dirty(display, x_min, y_min, x_max - x_min + 1, y_max - y_min + 1);
}

void ssd1306_draw_star(ssd1306_t* display, uint8_t center_x, uint8_t center_y, uint8_t outer_radius, bool color, bool filled) {
    if (!display || outer_radius == 0) return;
    
    // Calculate inner radius (typically 0.4 times outer radius for nice 5-point star)
    uint8_t inner_radius = (outer_radius * 2) / 5;
    
    // 5-point star: 10 points total (5 outer, 5 inner), 36 degrees between each
    int16_t points_x[10], points_y[10];
    
    for (int i = 0; i < 10; i++) {
        // Alternate between outer and inner points
        uint8_t radius = (i % 2 == 0) ? outer_radius : inner_radius;
        
        // Angle in degrees: 0, 36, 72, 108, 144, 180, 216, 252, 288, 324
        // Start at -90 degrees to point upward
        int angle = (i * 36) - 90;
        
        // Convert to radians and calculate points
        float rad = (angle * 3.14159) / 180.0;
        points_x[i] = center_x + (int16_t)(radius * cos(rad));
        points_y[i] = center_y + (int16_t)(radius * sin(rad));
    }
    
    if (filled) {
        // For filled star, we'll use a simple scanline approach
        // This is complex for arbitrary polygons, so let's do outline for now
        // and potentially enhance later if needed
        filled = false;  // Force outline mode for now
    }
    
    // Draw star outline by connecting the points
    for (int i = 0; i < 10; i++) {
        int next = (i + 1) % 10;
        
        // Draw line from point i to point next
        // Simple line drawing algorithm
        int dx = abs(points_x[next] - points_x[i]);
        int dy = abs(points_y[next] - points_y[i]);
        int sx = (points_x[i] < points_x[next]) ? 1 : -1;
        int sy = (points_y[i] < points_y[next]) ? 1 : -1;
        int err = dx - dy;
        
        int x = points_x[i];
        int y = points_y[i];
        
        while (true) {
            ssd1306_set_pixel(display, x, y, color);
            
            if (x == points_x[next] && y == points_y[next]) break;
            
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x += sx;
            }
            if (e2 < dx) {
                err += dx;
                y += sy;
            }
        }
    }
    
    // Auto-mark bounding box as dirty
    uint8_t x_min = (center_x > outer_radius) ? center_x - outer_radius : 0;
    uint8_t y_min = (center_y > outer_radius) ? center_y - outer_radius : 0;
    uint8_t x_max = (center_x + outer_radius < SSD1306_WIDTH) ? center_x + outer_radius : SSD1306_WIDTH - 1;
    uint8_t y_max = (center_y + outer_radius < SSD1306_HEIGHT) ? center_y + outer_radius : SSD1306_HEIGHT - 1;
    ssd1306_mark_dirty(display, x_min, y_min, x_max - x_min + 1, y_max - y_min + 1);
}

//============================================================================
// TEXT AND FONT FUNCTIONS
//============================================================================

void ssd1306_set_font(ssd1306_t* display, const ssd1306_font_t* font) {
    if (!display || !font) return;
    display->current_font = font;
}

uint8_t ssd1306_draw_char(ssd1306_t* display, uint8_t x, uint8_t y, char ch, bool color) {
    // Convert single byte character to UTF-8 and delegate to UTF-8 function
    uint8_t utf8_char[2] = {(uint8_t)ch, 0};
    return ssd1306_draw_utf8_char(display, x, y, utf8_char, color);
}

uint16_t ssd1306_draw_string(ssd1306_t* display, uint8_t x, uint8_t y, const char* str, bool color) {
    if (!display || !str) return 0;
    
    uint16_t total_width = 0;
    uint8_t current_x = x;
    
    while (*str && current_x < SSD1306_WIDTH) {
        uint8_t char_width = ssd1306_draw_char(display, current_x, y, *str, color);
        current_x += char_width + 1;  // Add 1 pixel spacing between characters
        total_width += char_width + 1;
        str++;
    }
    
    return total_width;
}

uint16_t ssd1306_string_width(ssd1306_t* display, const char* str) {
    if (!display || !str || !display->current_font) return 0;
    
    uint16_t width = 0;
    while (*str) {
        width += display->current_font->width + 1;  // Character width + spacing
        str++;
    }
    
    if (width > 0) width--;  // Remove last spacing
    return width;
}

//============================================================================
// UTILITY FUNCTIONS
//============================================================================

bool ssd1306_set_contrast(ssd1306_t* display, uint8_t contrast) {
    if (!display) return false;
    
    return ssd1306_send_command(display, SSD1306_SET_CONTRAST) &&
           ssd1306_send_command(display, contrast);
}

bool ssd1306_display_on(ssd1306_t* display) {
    if (!display) return false;
    return ssd1306_send_command(display, SSD1306_DISPLAY_ON);
}

bool ssd1306_display_off(ssd1306_t* display) {
    if (!display) return false;
    return ssd1306_send_command(display, SSD1306_DISPLAY_OFF);
}

bool ssd1306_invert_display(ssd1306_t* display, bool invert) {
    if (!display) return false;
    
    uint8_t cmd = invert ? SSD1306_DISPLAY_INVERT : SSD1306_DISPLAY_NORMAL;
    return ssd1306_send_command(display, cmd);
}


//============================================================================
// JAPANESE TEXT RENDERING FUNCTIONS
//============================================================================

uint8_t ssd1306_draw_utf8_char(ssd1306_t* display, uint8_t x, uint8_t y, const uint8_t* utf8_char, bool color) {
    if (!display || !display->current_font || !utf8_char) return 0;
    
    const ssd1306_font_t* font = display->current_font;
    
    // Get character index in font array
    uint8_t bytes_consumed;
    uint16_t char_index = japanese_char_to_index(utf8_char, &bytes_consumed);
    
    if (char_index == 0xFFFF) {
        // Character not found, draw replacement character (question mark at index 30)
        char_index = 30; // '?' in ASCII section
    }
    
    // Use Y coordinate directly (no baseline compensation)
    uint8_t render_y = y;
    
    // Draw character bitmap
    // Cast font_data to appropriate type based on font size
    if (font->width == 8 && font->height == 8) {
        // Misaki 8px font
        const uint8_t (*bitmap)[8] = (const uint8_t (*)[8])font->font_data;
        for (uint8_t row = 0; row < 8; row++) {
            uint8_t bitmap_byte = bitmap[char_index][row];
            for (uint8_t col = 0; col < 8; col++) {
                if (bitmap_byte & (1 << col)) {
                    ssd1306_set_pixel(display, x + col, render_y + row, color);
                }
            }
        }
    } else if (font->width == 10 && font->height == 13) {
        // PixelMplus 10px font (17 bytes per character = 136 bits for 130 pixels)
        const uint8_t (*bitmap)[17] = (const uint8_t (*)[17])font->font_data;
        
        // Extract 130 bits (10x13 pixels) from 17 bytes efficiently
        for (uint8_t row = 0; row < 13; row++) {
            for (uint8_t col = 0; col < 10; col++) {
                uint16_t bit_index = row * 10 + col;  // Pixel position in 130-pixel array
                uint8_t byte_index = bit_index / 8;   // Which byte contains this bit
                uint8_t bit_offset = bit_index % 8;   // Which bit within that byte
                
                if (byte_index < 17) {  // Safety check
                    uint8_t byte_value = bitmap[char_index][byte_index];
                    if (byte_value & (1 << bit_offset)) {
                        ssd1306_set_pixel(display, x + col, render_y + row, color);
                    }
                }
            }
        }
    } else if (font->width == 12 && font->height == 15) {
        // PixelMplus 12px font (23 bytes per character = 184 bits for 180 pixels)
        const uint8_t (*bitmap)[23] = (const uint8_t (*)[23])font->font_data;
        
        // Extract 180 bits (12x15 pixels) from 23 bytes efficiently
        for (uint8_t row = 0; row < 15; row++) {
            for (uint8_t col = 0; col < 12; col++) {
                uint16_t bit_index = row * 12 + col;  // Pixel position in 180-pixel array
                uint8_t byte_index = bit_index / 8;   // Which byte contains this bit
                uint8_t bit_offset = bit_index % 8;   // Which bit within that byte
                
                if (byte_index < 23) {  // Safety check
                    uint8_t byte_value = bitmap[char_index][byte_index];
                    if (byte_value & (1 << bit_offset)) {
                        ssd1306_set_pixel(display, x + col, render_y + row, color);
                    }
                }
            }
        }
    }
    
    // Auto-mark character region as dirty (includes descenders)
    ssd1306_mark_dirty(display, x, render_y, font->width, font->height);
    
    return font->width;
}

uint16_t ssd1306_draw_utf8_string(ssd1306_t* display, uint8_t x, uint8_t y, const char* utf8_str, bool color) {
    if (!display || !utf8_str) return 0;
    
    uint16_t total_width = 0;
    uint8_t current_x = x;
    const uint8_t* str = (const uint8_t*)utf8_str;
    
    while (*str && current_x < SSD1306_WIDTH) {
        uint8_t char_width = ssd1306_draw_utf8_char(display, current_x, y, str, color);
        
        // Get proper character width for spacing calculation
        uint8_t display_width = japanese_char_width(str, display->current_font);
        
        // Move to next character in UTF-8 string
        uint8_t bytes_consumed;
        japanese_char_to_index(str, &bytes_consumed);
        str += bytes_consumed;
        
        // Use display width for advancement (includes proper spacing)
        current_x += display_width;
        total_width += display_width;
    }
    
    return total_width;
}

uint16_t ssd1306_utf8_string_width(ssd1306_t* display, const char* utf8_str) {
    if (!display || !utf8_str || !display->current_font) return 0;
    
    uint16_t width = 0;
    const uint8_t* str = (const uint8_t*)utf8_str;
    
    while (*str) {
        // Use proper character width function for spacing
        width += japanese_char_width(str, display->current_font);
        
        // Move to next character in UTF-8 string
        uint8_t bytes_consumed;
        japanese_char_to_index(str, &bytes_consumed);
        str += bytes_consumed;
    }
    
    return width;
}

//============================================================================
// ANIMATION AND TIMING FUNCTIONS
//============================================================================

// Global frame rate setting
static uint8_t g_fps = 10;  // Default to 10 FPS

void ssd1306_set_fps(uint8_t fps) {
    if (fps >= 1 && fps <= 60) {
        g_fps = fps;
    }
}

uint8_t ssd1306_get_fps(void) {
    return g_fps;
}

uint32_t ssd1306_get_frame_delay_ms(void) {
    return 1000 / g_fps;
}

void ssd1306_frame_delay(void) {
    sleep_ms(ssd1306_get_frame_delay_ms());
}

//============================================================================
// PARTIAL DISPLAY UPDATE FUNCTIONS
//============================================================================

bool ssd1306_display_partial(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    if (!display || !display->initialized) return false;
    
    // Clamp to display bounds
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return false;
    if (x + width > SSD1306_WIDTH) width = SSD1306_WIDTH - x;
    if (y + height > SSD1306_HEIGHT) height = SSD1306_HEIGHT - y;
    
    // Convert Y coordinates to page boundaries (SSD1306 is page-oriented)
    uint8_t start_page = y / 8;
    uint8_t end_page = (y + height - 1) / 8;
    
    // Set column address range
    if (!ssd1306_send_command(display, SSD1306_SET_COLUMN_ADDR) ||
        !ssd1306_send_command(display, x) ||
        !ssd1306_send_command(display, x + width - 1)) {
        return false;
    }
    
    // Set page address range  
    if (!ssd1306_send_command(display, SSD1306_SET_PAGE_ADDR) ||
        !ssd1306_send_command(display, start_page) ||
        !ssd1306_send_command(display, end_page)) {
        return false;
    }
    
    // Send data for the specified region
    for (uint8_t page = start_page; page <= end_page; page++) {
        uint8_t data_buffer[width + 1];
        data_buffer[0] = 0x40;  // Data control byte
        
        // Copy buffer data for this page and column range
        for (uint8_t col = 0; col < width; col++) {
            uint16_t buffer_index = (page * SSD1306_WIDTH) + (x + col);
            data_buffer[col + 1] = display->buffer[buffer_index];
        }
        
        int result = i2c_write_timeout_us(display->i2c_port, SSD1306_I2C_ADDRESS, 
                                         data_buffer, width + 1, false, 50000);
        if (result != width + 1) {
            return false;
        }
    }
    
    return true;
}

void ssd1306_mark_dirty(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    if (!display) return;
    
    // Clamp to display bounds
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    if (x + width > SSD1306_WIDTH) width = SSD1306_WIDTH - x;
    if (y + height > SSD1306_HEIGHT) height = SSD1306_HEIGHT - y;
    
    if (!display->has_dirty_region) {
        // First dirty region
        display->dirty_x_min = x;
        display->dirty_y_min = y;
        display->dirty_x_max = x + width - 1;
        display->dirty_y_max = y + height - 1;
        display->has_dirty_region = true;
    } else {
        // Expand existing dirty region
        if (x < display->dirty_x_min) display->dirty_x_min = x;
        if (y < display->dirty_y_min) display->dirty_y_min = y;
        if (x + width - 1 > display->dirty_x_max) display->dirty_x_max = x + width - 1;
        if (y + height - 1 > display->dirty_y_max) display->dirty_y_max = y + height - 1;
    }
}

bool ssd1306_display_dirty_regions(ssd1306_t* display) {
    if (!display || !display->has_dirty_region) return false;
    
    uint8_t width = display->dirty_x_max - display->dirty_x_min + 1;
    uint8_t height = display->dirty_y_max - display->dirty_y_min + 1;
    
    // Intelligent update strategy: choose partial vs full update
    uint16_t dirty_pixels = width * height;
    uint16_t total_pixels = SSD1306_WIDTH * SSD1306_HEIGHT;
    uint8_t dirty_percentage = (dirty_pixels * 100) / total_pixels;
    
    bool result;
    
    // Decision logic for update strategy
    if (dirty_percentage >= 75) {
        // Large dirty region: use full update (more efficient for big changes)
        result = ssd1306_display(display);
    } else if (width >= 120 && height >= 50) {
        // Wide, tall region: use full update (partial update overhead not worth it)
        result = ssd1306_display(display);
    } else {
        // Small/medium dirty region: use partial update (much more efficient)
        result = ssd1306_display_partial(display, 
                                         display->dirty_x_min, display->dirty_y_min,
                                         width, height);
    }
    
    // Clear dirty region
    display->has_dirty_region = false;
    
    return result;
}

void ssd1306_set_priority(ssd1306_t* display, uint8_t priority) {
    if (display) {
        display->update_priority = priority;
    }
}

bool ssd1306_get_dirty_info(ssd1306_t* display, uint8_t* width, uint8_t* height, uint16_t* pixels) {
    if (!display || !display->has_dirty_region) return false;
    
    uint8_t w = display->dirty_x_max - display->dirty_x_min + 1;
    uint8_t h = display->dirty_y_max - display->dirty_y_min + 1;
    
    if (width) *width = w;
    if (height) *height = h;
    if (pixels) *pixels = w * h;
    
    return true;
}

//============================================================================
// INVERSION AND XOR FUNCTIONS
//============================================================================

void ssd1306_xor_pixel(ssd1306_t* display, uint8_t x, uint8_t y) {
    if (!display || x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    
    uint16_t index = x + (y / 8) * SSD1306_WIDTH;
    uint8_t bit = y % 8;
    
    // XOR the pixel (toggle its state)
    display->buffer[index] ^= (1 << bit);
}

void ssd1306_invert_region(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    if (!display) return;
    
    // Clamp to display bounds
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    if (x + width > SSD1306_WIDTH) width = SSD1306_WIDTH - x;
    if (y + height > SSD1306_HEIGHT) height = SSD1306_HEIGHT - y;
    
    // Invert each pixel in the region using XOR
    for (uint8_t j = 0; j < height; j++) {
        for (uint8_t i = 0; i < width; i++) {
            ssd1306_xor_pixel(display, x + i, y + j);
        }
    }
    
    // Auto-mark region as dirty
    ssd1306_mark_dirty(display, x, y, width, height);
}

uint8_t ssd1306_draw_utf8_char_xor(ssd1306_t* display, uint8_t x, uint8_t y, const uint8_t* utf8_char, bool color) {
    if (!display || !display->current_font || !utf8_char) return 0;
    
    const ssd1306_font_t* font = display->current_font;
    
    // Get character index in font array
    uint8_t bytes_consumed;
    uint16_t char_index = japanese_char_to_index(utf8_char, &bytes_consumed);
    
    if (char_index == 0xFFFF) {
        // Character not found, draw replacement character (question mark at index 30)
        char_index = 30; // '?' in ASCII section
    }
    
    // Use Y coordinate directly (no baseline compensation)
    uint8_t render_y = y;
    
    // Draw character bitmap using XOR operations
    // Cast font_data to appropriate type based on font size
    if (font->width == 8 && font->height == 8) {
        // Misaki 8px font
        const uint8_t (*bitmap)[8] = (const uint8_t (*)[8])font->font_data;
        for (uint8_t row = 0; row < 8; row++) {
            uint8_t bitmap_byte = bitmap[char_index][row];
            for (uint8_t col = 0; col < 8; col++) {
                if (bitmap_byte & (1 << col)) {
                    ssd1306_xor_pixel(display, x + col, render_y + row);
                }
            }
        }
    } else if (font->width == 10 && font->height == 13) {
        // PixelMplus 10px font (17 bytes per character = 136 bits for 130 pixels)
        const uint8_t (*bitmap)[17] = (const uint8_t (*)[17])font->font_data;
        
        // Extract 130 bits (10x13 pixels) from 17 bytes efficiently
        for (uint8_t row = 0; row < 13; row++) {
            for (uint8_t col = 0; col < 10; col++) {
                uint16_t bit_index = row * 10 + col;  // Pixel position in 130-pixel array
                uint8_t byte_index = bit_index / 8;   // Which byte contains this bit
                uint8_t bit_offset = bit_index % 8;   // Which bit within that byte
                
                if (byte_index < 17) {  // Safety check
                    uint8_t byte_value = bitmap[char_index][byte_index];
                    if (byte_value & (1 << bit_offset)) {
                        ssd1306_xor_pixel(display, x + col, render_y + row);
                    }
                }
            }
        }
    } else if (font->width == 12 && font->height == 15) {
        // PixelMplus 12px font (23 bytes per character = 184 bits for 180 pixels)
        const uint8_t (*bitmap)[23] = (const uint8_t (*)[23])font->font_data;
        
        // Extract 180 bits (12x15 pixels) from 23 bytes efficiently
        for (uint8_t row = 0; row < 15; row++) {
            for (uint8_t col = 0; col < 12; col++) {
                uint16_t bit_index = row * 12 + col;  // Pixel position in 180-pixel array
                uint8_t byte_index = bit_index / 8;   // Which byte contains this bit
                uint8_t bit_offset = bit_index % 8;   // Which bit within that byte
                
                if (byte_index < 23) {  // Safety check
                    uint8_t byte_value = bitmap[char_index][byte_index];
                    if (byte_value & (1 << bit_offset)) {
                        ssd1306_xor_pixel(display, x + col, render_y + row);
                    }
                }
            }
        }
    }
    
    // Auto-mark character region as dirty (includes descenders)
    ssd1306_mark_dirty(display, x, render_y, font->width, font->height);
    
    return font->width;
}

uint16_t ssd1306_draw_utf8_string_inverted(ssd1306_t* display, uint8_t x, uint8_t y, const char* utf8_str) {
    if (!display || !utf8_str) return 0;
    
    uint16_t total_width = 0;
    uint8_t current_x = x;
    const uint8_t* str = (const uint8_t*)utf8_str;
    
    while (*str && current_x < SSD1306_WIDTH) {
        uint8_t char_width = ssd1306_draw_utf8_char_xor(display, current_x, y, str, true);
        
        // Get proper character width for spacing calculation
        uint8_t display_width = japanese_char_width(str, display->current_font);
        
        // Move to next character in UTF-8 string
        uint8_t bytes_consumed;
        japanese_char_to_index(str, &bytes_consumed);
        str += bytes_consumed;
        
        // Use display width for advancement (includes proper spacing)
        current_x += display_width;
        total_width += display_width;
    }
    
    return total_width;
}

//============================================================================
// PROGRESS BAR AND INDICATOR FUNCTIONS
//============================================================================

void ssd1306_draw_progress_bar(ssd1306_t* display, uint8_t x, uint8_t y, 
                               uint8_t width, uint8_t height, 
                               uint8_t progress, uint8_t max_progress) {
    if (!display || width == 0 || height == 0 || max_progress == 0) return;
    
    // Draw outline rectangle (auto-marks dirty)
    ssd1306_draw_rect(display, x, y, width, height, true, false);
    
    // Calculate fill width (leave 2px border)
    uint8_t fill_width = 0;
    if (progress > 0 && width > 2) {
        fill_width = ((progress * (width - 2)) / max_progress);
        if (fill_width > width - 2) fill_width = width - 2;
    }
    
    // Clear interior first (auto-marks dirty)
    if (width > 2 && height > 2) {
        ssd1306_draw_rect(display, x + 1, y + 1, width - 2, height - 2, false, true);
    }
    
    // Draw filled portion (auto-marks dirty)
    if (fill_width > 0 && height > 2) {
        ssd1306_draw_rect(display, x + 1, y + 1, fill_width, height - 2, true, true);
    }
    
    // Note: Individual draw_rect calls auto-mark dirty, so entire progress bar is covered
}

void ssd1306_draw_indicator_bar(ssd1306_t* display, uint8_t x, uint8_t y,
                                uint8_t width, uint8_t height,
                                uint8_t value, uint8_t min_val, uint8_t max_val) {
    if (!display || width == 0 || height == 0 || max_val <= min_val) return;
    
    // Clamp value to range
    if (value < min_val) value = min_val;
    if (value > max_val) value = max_val;
    
    // Convert to progress (0 to range)
    uint8_t range = max_val - min_val;
    uint8_t progress = value - min_val;
    
    // Use progress bar implementation
    ssd1306_draw_progress_bar(display, x, y, width, height, progress, range);
}

void ssd1306_draw_styled_bar(ssd1306_t* display, uint8_t x, uint8_t y,
                             uint8_t width, uint8_t height,
                             uint8_t value, uint8_t max_value,
                             ssd1306_bar_style_t style) {
    if (!display || width == 0 || height == 0 || max_value == 0) return;
    
    switch (style) {
        case BAR_STYLE_FILLED:
            ssd1306_draw_progress_bar(display, x, y, width, height, value, max_value);
            break;
            
        case BAR_STYLE_OUTLINED:
            // Just draw the outline
            ssd1306_draw_rect(display, x, y, width, height, true, false);
            break;
            
        case BAR_STYLE_SEGMENTED: {
            // Draw segmented bar with gaps
            ssd1306_draw_rect(display, x, y, width, height, true, false);
            
            if (width > 6 && height > 2 && value > 0) {
                // Calculate segment parameters
                uint8_t segment_count = (width - 2) / 4;  // 3px segments + 1px gap
                uint8_t filled_segments = (value * segment_count) / max_value;
                
                // Draw segments
                for (uint8_t i = 0; i < filled_segments && i < segment_count; i++) {
                    uint8_t seg_x = x + 1 + (i * 4);
                    if (seg_x + 3 <= x + width - 1) {
                        ssd1306_draw_rect(display, seg_x, y + 1, 3, height - 2, true, true);
                    }
                }
            }
            break;
        }
    }
}

//============================================================================
// TEXT ANIMATION FUNCTIONS (Build Effects)
//============================================================================

uint16_t ssd1306_utf8_char_count(const char* utf8_str) {
    if (!utf8_str) return 0;
    
    uint16_t char_count = 0;
    const char* ptr = utf8_str;
    
    while (*ptr) {
        if ((*ptr & 0x80) == 0) {
            // ASCII character (0xxxxxxx)
            ptr += 1;
        } else if ((*ptr & 0xE0) == 0xC0) {
            // 2-byte character (110xxxxx)
            ptr += 2;
        } else if ((*ptr & 0xF0) == 0xE0) {
            // 3-byte character (1110xxxx) - Japanese characters
            ptr += 3;
        } else if ((*ptr & 0xF8) == 0xF0) {
            // 4-byte character (11110xxx)
            ptr += 4;
        } else {
            // Invalid UTF-8, skip byte
            ptr += 1;
        }
        char_count++;
    }
    
    return char_count;
}

uint16_t ssd1306_utf8_substring(const char* utf8_str, uint16_t char_count, char* buffer, uint16_t buffer_size) {
    if (!utf8_str || !buffer || buffer_size == 0) return 0;
    
    const char* ptr = utf8_str;
    uint16_t bytes_written = 0;
    uint16_t chars_processed = 0;
    
    while (*ptr && chars_processed < char_count && bytes_written < buffer_size - 1) {
        uint8_t char_bytes = 1;
        
        if ((*ptr & 0x80) == 0) {
            // ASCII character
            char_bytes = 1;
        } else if ((*ptr & 0xE0) == 0xC0) {
            // 2-byte character
            char_bytes = 2;
        } else if ((*ptr & 0xF0) == 0xE0) {
            // 3-byte character (Japanese)
            char_bytes = 3;
        } else if ((*ptr & 0xF8) == 0xF0) {
            // 4-byte character
            char_bytes = 4;
        }
        
        // Check if we have enough space for this character
        if (bytes_written + char_bytes >= buffer_size) break;
        
        // Copy character bytes
        for (uint8_t i = 0; i < char_bytes && *ptr; i++) {
            buffer[bytes_written++] = *ptr++;
        }
        
        chars_processed++;
    }
    
    buffer[bytes_written] = '\0';
    return bytes_written;
}

uint16_t ssd1306_draw_utf8_string_typewriter(ssd1306_t* display, uint8_t x, uint8_t y, 
                                             const char* utf8_str, uint32_t progress_ms, 
                                             uint32_t char_interval_ms, bool show_cursor) {
    if (!display || !utf8_str) return 0;
    
    uint16_t total_chars = ssd1306_utf8_char_count(utf8_str);
    uint8_t cursor_height = display->current_font ? display->current_font->height : 8;
    uint8_t cursor_width = display->current_font ? (display->current_font->width > 8 ? 8 : display->current_font->width) : 6;
    
    // Animation phases:
    // Phase 1: Show cursor and blink twice (1000ms total: 250ms on, 250ms off, 250ms on, 250ms off)
    // Phase 2: Type characters (char_interval_ms per character)  
    // Phase 3: Keep cursor and blink twice (1000ms total: 250ms on, 250ms off, 250ms on, 250ms off)
    // Phase 4: Remove cursor permanently
    
    uint32_t phase1_duration = 1000; // Initial cursor blinks
    uint32_t phase2_duration = total_chars * char_interval_ms; // Typing
    uint32_t phase3_duration = 1000; // Final cursor blinks
    
    uint32_t phase1_end = phase1_duration;
    uint32_t phase2_end = phase1_end + phase2_duration;
    uint32_t phase3_end = phase2_end + phase3_duration;
    
    uint16_t visible_chars = 0;
    bool draw_cursor = false;
    
    if (progress_ms < phase1_end) {
        // Phase 1: Initial cursor blinking (no text yet)
        visible_chars = 0;
        uint32_t blink_cycle = (progress_ms / 250) % 4; // 4 phases: on, off, on, off
        draw_cursor = show_cursor && (blink_cycle == 0 || blink_cycle == 2); // Show during 1st and 3rd quarters
        
    } else if (progress_ms < phase2_end) {
        // Phase 2: Typing characters
        uint32_t typing_elapsed = progress_ms - phase1_end;
        visible_chars = typing_elapsed / char_interval_ms;
        if (visible_chars > total_chars) visible_chars = total_chars;
        
        // Show cursor while typing (solid, no blinking)
        draw_cursor = show_cursor && (visible_chars < total_chars);
        
    } else if (progress_ms < phase3_end) {
        // Phase 3: All text shown, cursor blinking at end
        visible_chars = total_chars;
        uint32_t final_blink_elapsed = progress_ms - phase2_end;
        uint32_t blink_cycle = (final_blink_elapsed / 250) % 4; // 4 phases: on, off, on, off
        draw_cursor = show_cursor && (blink_cycle == 0 || blink_cycle == 2); // Show during 1st and 3rd quarters
        
    } else {
        // Phase 4: Animation complete, no cursor
        visible_chars = total_chars;
        draw_cursor = false;
    }
    
    // Extract substring with correct number of UTF-8 characters
    char partial_text[128] = {0};
    ssd1306_utf8_substring(utf8_str, visible_chars, partial_text, sizeof(partial_text));
    
    // Draw the visible portion
    uint16_t text_width = ssd1306_draw_utf8_string(display, x, y, partial_text, true);
    
    // Draw block cursor if needed
    if (draw_cursor) {
        uint16_t cursor_x = x + text_width;
        ssd1306_draw_rect(display, cursor_x, y, cursor_width, cursor_height, true, true);
    }
    
    // Return total width that would be drawn when complete
    return ssd1306_utf8_string_width(display, utf8_str);
}