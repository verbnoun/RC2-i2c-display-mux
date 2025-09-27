//============================================================================
// SSD1306 Graphics Library Implementation
// High-level graphics and text rendering for SSD1306 OLED displays
//============================================================================

#include "ssd1306_graphics.h"
#include <string.h>

//============================================================================
// CORE FUNCTIONS
//============================================================================

bool ssd1306_init(ssd1306_t* display, i2c_inst_t* i2c_port) {
    if (!display || !i2c_port) return false;
    
    display->i2c_port = i2c_port;
    display->initialized = false;
    display->current_font = &font_6x8;  // Default font
    
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
}

void ssd1306_fill(ssd1306_t* display) {
    if (!display) return;
    memset(display->buffer, 0xFF, SSD1306_BUFFER_SIZE);
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
}

void ssd1306_draw_vline(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t height, bool color) {
    if (!display) return;
    
    for (uint8_t i = 0; i < height && (y + i) < SSD1306_HEIGHT; i++) {
        ssd1306_set_pixel(display, x, y + i, color);
    }
}

void ssd1306_draw_rect(ssd1306_t* display, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool color, bool filled) {
    if (!display) return;
    
    if (filled) {
        for (uint8_t j = 0; j < height && (y + j) < SSD1306_HEIGHT; j++) {
            ssd1306_draw_hline(display, x, y + j, width, color);
        }
    } else {
        // Draw outline
        ssd1306_draw_hline(display, x, y, width, color);                    // Top
        ssd1306_draw_hline(display, x, y + height - 1, width, color);      // Bottom
        ssd1306_draw_vline(display, x, y, height, color);                  // Left
        ssd1306_draw_vline(display, x + width - 1, y, height, color);      // Right
    }
}

//============================================================================
// TEXT AND FONT FUNCTIONS
//============================================================================

void ssd1306_set_font(ssd1306_t* display, const ssd1306_font_t* font) {
    if (!display || !font) return;
    display->current_font = font;
}

uint8_t ssd1306_draw_char(ssd1306_t* display, uint8_t x, uint8_t y, char ch, bool color) {
    if (!display || !display->current_font) return 0;
    
    const ssd1306_font_t* font = display->current_font;
    
    // Check if character is in font range
    if (ch < font->first_char || ch > font->last_char) {
        ch = '?';  // Replace with question mark if not available
        if (ch < font->first_char || ch > font->last_char) {
            return font->width;  // Skip if even question mark not available
        }
    }
    
    // For 8x16 fonts using 2D array format
    if (font->height == 16 && font->width == 8) {
        // Draw character from 2D array font[char][row]
        for (uint8_t row = 0; row < 16; row++) {
            if (y + row >= SSD1306_HEIGHT) break;
            
            uint8_t row_data = font->font_data[ch][row];
            
            for (uint8_t col = 0; col < 8; col++) {
                if (x + col >= SSD1306_WIDTH) break;
                
                // Check if bit is set (MSB = leftmost pixel)
                bool pixel_on = (row_data & (0x80 >> col)) != 0;
                if (pixel_on) {
                    ssd1306_set_pixel(display, x + col, y + row, color);
                }
            }
        }
    }
    
    return font->width;
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