//============================================================================
// Adafruit GFX Graphics Library - Pico SDK Port
// Core graphics primitives and text rendering
// Based on Adafruit GFX Library
//============================================================================

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <cstddef>

//============================================================================
// CONSTANTS
//============================================================================

#define SH110X_BLACK 0   ///< Draw 'off' pixels
#define SH110X_WHITE 1   ///< Draw 'on' pixels
#define SH110X_INVERSE 2 ///< Invert pixels

//============================================================================
// FONT STRUCTURE
//============================================================================

typedef struct {
    uint8_t *bitmap;      // Glyph bitmaps, concatenated
    uint16_t *glyph;      // Glyph array
    uint8_t first;        // ASCII extents (first char)
    uint8_t last;         // ASCII extents (last char)
    uint8_t yAdvance;     // Newline distance (y axis)
} GFXfont;

//============================================================================
// ADAFRUIT GFX BASE CLASS
//============================================================================

class Adafruit_GFX {
public:
    // Constructor
    Adafruit_GFX(int16_t w, int16_t h);
    virtual ~Adafruit_GFX() {}

    // Pure virtual function - must be implemented by display driver
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;

    // Virtual functions that can be overridden for optimization
    virtual void startWrite(void) {}
    virtual void endWrite(void) {}
    virtual void writePixel(int16_t x, int16_t y, uint16_t color) { drawPixel(x, y, color); }
    virtual void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    virtual void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    // Basic drawing primitives
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillScreen(uint16_t color);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

    // Bitmap drawing
    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);
    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg);

    // Text rendering
    void setCursor(int16_t x, int16_t y) { cursor_x = x; cursor_y = y; }
    void setTextColor(uint16_t c) { textcolor = textbgcolor = c; }
    void setTextColor(uint16_t c, uint16_t bg) { textcolor = c; textbgcolor = bg; }
    void setTextSize(uint8_t s) { textsize = s; }
    void setTextWrap(bool w) { wrap = w; }
    void setFont(const GFXfont *f = nullptr) { gfxFont = (GFXfont *)f; }

    // Print interface
    size_t write(uint8_t c);
    size_t print(const char *str);
    size_t println(const char *str);

    // Character and string bounds
    void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void charBounds(unsigned char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy);

    // Rotation and clipping
    void setRotation(uint8_t r);
    uint8_t getRotation(void) const { return rotation; }

    // Dimension getters
    int16_t width(void) const { return _width; }
    int16_t height(void) const { return _height; }
    int16_t getCursorX(void) const { return cursor_x; }
    int16_t getCursorY(void) const { return cursor_y; }

protected:
    // Screen dimensions
    int16_t WIDTH;        ///< This is the 'raw' display width - never changes
    int16_t HEIGHT;       ///< This is the 'raw' display height - never changes
    int16_t _width;       ///< Display width as modified by current rotation
    int16_t _height;      ///< Display height as modified by current rotation

    // Text cursor state
    int16_t cursor_x;     ///< x location to start print()ing text
    int16_t cursor_y;     ///< y location to start print()ing text
    uint16_t textcolor;   ///< 16-bit background color for print()
    uint16_t textbgcolor; ///< 16-bit text color for print()
    uint8_t textsize;     ///< Desired magnification of text to print()
    uint8_t rotation;     ///< Display rotation (0 thru 3)
    bool wrap;            ///< If set, 'wrap' text at right edge of display
    GFXfont *gfxFont;     ///< Pointer to special font

    // Helper functions
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
};

//============================================================================
// BUILT-IN 5x8 FONT
//============================================================================

extern const uint8_t font[];  // 5x8 bitmap font array