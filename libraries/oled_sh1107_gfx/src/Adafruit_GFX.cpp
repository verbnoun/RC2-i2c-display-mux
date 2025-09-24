//============================================================================
// Adafruit GFX Graphics Library Implementation - Pico SDK Port
// Core graphics primitives and text rendering
//============================================================================

#include "Adafruit_GFX.h"
#include <string.h>
#include <algorithm>

//============================================================================
// 5x8 BITMAP FONT (basic ASCII characters 32-126)
//============================================================================

const uint8_t font[] = {
    0x00, 0x00, 0x00, 0x00, 0x00,   // 32 (space)
    0x00, 0x00, 0x5f, 0x00, 0x00,   // 33 !
    0x00, 0x07, 0x00, 0x07, 0x00,   // 34 "
    0x14, 0x7f, 0x14, 0x7f, 0x14,   // 35 #
    0x24, 0x2a, 0x7f, 0x2a, 0x12,   // 36 $
    0x23, 0x13, 0x08, 0x64, 0x62,   // 37 %
    0x36, 0x49, 0x55, 0x22, 0x50,   // 38 &
    0x00, 0x05, 0x03, 0x00, 0x00,   // 39 '
    0x00, 0x1c, 0x22, 0x41, 0x00,   // 40 (
    0x00, 0x41, 0x22, 0x1c, 0x00,   // 41 )
    0x14, 0x08, 0x3e, 0x08, 0x14,   // 42 *
    0x08, 0x08, 0x3e, 0x08, 0x08,   // 43 +
    0x00, 0x50, 0x30, 0x00, 0x00,   // 44 ,
    0x08, 0x08, 0x08, 0x08, 0x08,   // 45 -
    0x00, 0x60, 0x60, 0x00, 0x00,   // 46 .
    0x20, 0x10, 0x08, 0x04, 0x02,   // 47 /
    0x3e, 0x51, 0x49, 0x45, 0x3e,   // 48 0
    0x00, 0x42, 0x7f, 0x40, 0x00,   // 49 1
    0x42, 0x61, 0x51, 0x49, 0x46,   // 50 2
    0x21, 0x41, 0x45, 0x4b, 0x31,   // 51 3
    0x18, 0x14, 0x12, 0x7f, 0x10,   // 52 4
    0x27, 0x45, 0x45, 0x45, 0x39,   // 53 5
    0x3c, 0x4a, 0x49, 0x49, 0x30,   // 54 6
    0x01, 0x71, 0x09, 0x05, 0x03,   // 55 7
    0x36, 0x49, 0x49, 0x49, 0x36,   // 56 8
    0x06, 0x49, 0x49, 0x29, 0x1e,   // 57 9
    0x00, 0x36, 0x36, 0x00, 0x00,   // 58 :
    0x00, 0x56, 0x36, 0x00, 0x00,   // 59 ;
    0x08, 0x14, 0x22, 0x41, 0x00,   // 60 <
    0x14, 0x14, 0x14, 0x14, 0x14,   // 61 =
    0x00, 0x41, 0x22, 0x14, 0x08,   // 62 >
    0x02, 0x01, 0x51, 0x09, 0x06,   // 63 ?
    0x32, 0x49, 0x59, 0x51, 0x3e,   // 64 @
    0x7e, 0x11, 0x11, 0x11, 0x7e,   // 65 A
    0x7f, 0x49, 0x49, 0x49, 0x36,   // 66 B
    0x3e, 0x41, 0x41, 0x41, 0x22,   // 67 C
    0x7f, 0x41, 0x41, 0x22, 0x1c,   // 68 D
    0x7f, 0x49, 0x49, 0x49, 0x41,   // 69 E
    0x7f, 0x09, 0x09, 0x09, 0x01,   // 70 F
    0x3e, 0x41, 0x49, 0x49, 0x7a,   // 71 G
    0x7f, 0x08, 0x08, 0x08, 0x7f,   // 72 H
    0x00, 0x41, 0x7f, 0x41, 0x00,   // 73 I
    0x20, 0x40, 0x41, 0x3f, 0x01,   // 74 J
    0x7f, 0x08, 0x14, 0x22, 0x41,   // 75 K
    0x7f, 0x40, 0x40, 0x40, 0x40,   // 76 L
    0x7f, 0x02, 0x0c, 0x02, 0x7f,   // 77 M
    0x7f, 0x04, 0x08, 0x10, 0x7f,   // 78 N
    0x3e, 0x41, 0x41, 0x41, 0x3e,   // 79 O
    0x7f, 0x09, 0x09, 0x09, 0x06,   // 80 P
    0x3e, 0x41, 0x51, 0x21, 0x5e,   // 81 Q
    0x7f, 0x09, 0x19, 0x29, 0x46,   // 82 R
    0x46, 0x49, 0x49, 0x49, 0x31,   // 83 S
    0x01, 0x01, 0x7f, 0x01, 0x01,   // 84 T
    0x3f, 0x40, 0x40, 0x40, 0x3f,   // 85 U
    0x1f, 0x20, 0x40, 0x20, 0x1f,   // 86 V
    0x3f, 0x40, 0x38, 0x40, 0x3f,   // 87 W
    0x63, 0x14, 0x08, 0x14, 0x63,   // 88 X
    0x07, 0x08, 0x70, 0x08, 0x07,   // 89 Y
    0x61, 0x51, 0x49, 0x45, 0x43,   // 90 Z
    0x00, 0x7f, 0x41, 0x41, 0x00,   // 91 [
    0x02, 0x04, 0x08, 0x10, 0x20,   // 92 backslash
    0x00, 0x41, 0x41, 0x7f, 0x00,   // 93 ]
    0x04, 0x02, 0x01, 0x02, 0x04,   // 94 ^
    0x40, 0x40, 0x40, 0x40, 0x40,   // 95 _
    0x00, 0x01, 0x02, 0x04, 0x00,   // 96 `
    0x20, 0x54, 0x54, 0x54, 0x78,   // 97 a
    0x7f, 0x48, 0x44, 0x44, 0x38,   // 98 b
    0x38, 0x44, 0x44, 0x44, 0x20,   // 99 c
    0x38, 0x44, 0x44, 0x48, 0x7f,   // 100 d
    0x38, 0x54, 0x54, 0x54, 0x18,   // 101 e
    0x08, 0x7e, 0x09, 0x01, 0x02,   // 102 f
    0x0c, 0x52, 0x52, 0x52, 0x3e,   // 103 g
    0x7f, 0x08, 0x04, 0x04, 0x78,   // 104 h
    0x00, 0x44, 0x7d, 0x40, 0x00,   // 105 i
    0x20, 0x40, 0x44, 0x3d, 0x00,   // 106 j
    0x7f, 0x10, 0x28, 0x44, 0x00,   // 107 k
    0x00, 0x41, 0x7f, 0x40, 0x00,   // 108 l
    0x7c, 0x04, 0x18, 0x04, 0x78,   // 109 m
    0x7c, 0x08, 0x04, 0x04, 0x78,   // 110 n
    0x38, 0x44, 0x44, 0x44, 0x38,   // 111 o
    0x7c, 0x14, 0x14, 0x14, 0x08,   // 112 p
    0x08, 0x14, 0x14, 0x18, 0x7c,   // 113 q
    0x7c, 0x08, 0x04, 0x04, 0x08,   // 114 r
    0x48, 0x54, 0x54, 0x54, 0x20,   // 115 s
    0x04, 0x3f, 0x44, 0x40, 0x20,   // 116 t
    0x3c, 0x40, 0x40, 0x20, 0x7c,   // 117 u
    0x1c, 0x20, 0x40, 0x20, 0x1c,   // 118 v
    0x3c, 0x40, 0x30, 0x40, 0x3c,   // 119 w
    0x44, 0x28, 0x10, 0x28, 0x44,   // 120 x
    0x0c, 0x50, 0x50, 0x50, 0x3c,   // 121 y
    0x44, 0x64, 0x54, 0x4c, 0x44,   // 122 z
    0x00, 0x08, 0x36, 0x41, 0x00,   // 123 {
    0x00, 0x00, 0x7f, 0x00, 0x00,   // 124 |
    0x00, 0x41, 0x36, 0x08, 0x00,   // 125 }
    0x10, 0x08, 0x08, 0x10, 0x08,   // 126 ~
};

//============================================================================
// CONSTRUCTOR
//============================================================================

Adafruit_GFX::Adafruit_GFX(int16_t w, int16_t h) : WIDTH(w), HEIGHT(h) {
    _width = WIDTH;
    _height = HEIGHT;
    rotation = 0;
    cursor_y = cursor_x = 0;
    textsize = 1;
    textcolor = textbgcolor = SH110X_WHITE;
    wrap = true;
    gfxFont = nullptr;
}

//============================================================================
// BASIC DRAWING PRIMITIVES
//============================================================================

void Adafruit_GFX::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            drawPixel(y0, x0, color);
        } else {
            drawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void Adafruit_GFX::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    startWrite();
    writeLine(x, y, x + w - 1, y, color);
    writeLine(x, y + h - 1, x + w - 1, y + h - 1, color);
    writeLine(x, y, x, y + h - 1, color);
    writeLine(x + w - 1, y, x + w - 1, y + h - 1, color);
    endWrite();
}

void Adafruit_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    startWrite();
    for (int16_t i = x; i < x + w; i++) {
        writeLine(i, y, i, y + h - 1, color);
    }
    endWrite();
}

void Adafruit_GFX::fillScreen(uint16_t color) {
    fillRect(0, 0, _width, _height, color);
}

void Adafruit_GFX::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    startWrite();
    writePixel(x0, y0 + r, color);
    writePixel(x0, y0 - r, color);
    writePixel(x0 + r, y0, color);
    writePixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        writePixel(x0 + x, y0 + y, color);
        writePixel(x0 - x, y0 + y, color);
        writePixel(x0 + x, y0 - y, color);
        writePixel(x0 - x, y0 - y, color);
        writePixel(x0 + y, y0 + x, color);
        writePixel(x0 - y, y0 + x, color);
        writePixel(x0 + y, y0 - x, color);
        writePixel(x0 - y, y0 - x, color);
    }
    endWrite();
}

void Adafruit_GFX::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    startWrite();
    writeLine(x0, y0 - r, x0, y0 + r + 1, color);
    
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        writeLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
        writeLine(x0 - x, y0 - y, x0 + x, y0 - y, color);
        writeLine(x0 - y, y0 + x, x0 + y, y0 + x, color);
        writeLine(x0 - y, y0 - x, x0 + y, y0 - x, color);
    }
    endWrite();
}

//============================================================================
// TEXT RENDERING
//============================================================================

void Adafruit_GFX::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color,
                            uint16_t bg, uint8_t size) {
    if ((x >= _width) || (y >= _height) || ((x + 6 * size - 1) < 0) || ((y + 8 * size - 1) < 0))
        return;

    if (!gfxFont) { // Use built-in font
        if (c >= 32 && c <= 126) {
            c -= 32; // Make it zero-indexed
            startWrite();
            for (int8_t i = 0; i < 5; i++) { // Char bitmap = 5 columns
                uint8_t line = font[c * 5 + i];
                for (int8_t j = 0; j < 8; j++, line >>= 1) {
                    if (line & 1) {
                        if (size == 1)
                            writePixel(x + i, y + j, color);
                        else
                            writeFillRect(x + i * size, y + j * size, size, size, color);
                    } else if (bg != color) {
                        if (size == 1)
                            writePixel(x + i, y + j, bg);
                        else
                            writeFillRect(x + i * size, y + j * size, size, size, bg);
                    }
                }
            }
            if (bg != color) { // If opaque, draw vertical line for last column
                if (size == 1)
                    writeLine(x + 5, y, x + 5, y + 8 - 1, bg);
                else
                    writeFillRect(x + 5 * size, y, size, 8 * size, bg);
            }
            endWrite();
        }
    }
    // TODO: Add custom font support
}

size_t Adafruit_GFX::write(uint8_t c) {
    if (!gfxFont) { // Built-in font
        if (c == '\n') {              // Newline?
            cursor_x = 0;             // Reset x to zero,
            cursor_y += textsize * 8; // advance y one line
        } else if (c != '\r') {       // Ignore carriage returns
            if (wrap && ((cursor_x + textsize * 6) > _width)) { // Off right?
                cursor_x = 0;                 // Reset x to zero,
                cursor_y += textsize * 8;     // advance y one line
            }
            drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
            cursor_x += textsize * 6; // Advance x one char
        }
    }
    // TODO: Add custom font support
    return 1;
}

size_t Adafruit_GFX::print(const char *str) {
    size_t n = 0;
    while (*str) {
        n += write(*str++);
    }
    return n;
}

size_t Adafruit_GFX::println(const char *str) {
    size_t n = print(str);
    n += write('\n');
    return n;
}

//============================================================================
// BITMAP DRAWING
//============================================================================

void Adafruit_GFX::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[],
                              int16_t w, int16_t h, uint16_t color) {
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    startWrite();
    for (int16_t j = 0; j < h; j++, y++) {
        for (int16_t i = 0; i < w; i++) {
            if (i & 7)
                byte <<= 1;
            else
                byte = bitmap[j * byteWidth + i / 8];
            if (byte & 0x80)
                writePixel(x + i, y, color);
        }
    }
    endWrite();
}

void Adafruit_GFX::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[],
                              int16_t w, int16_t h, uint16_t color, uint16_t bg) {
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    startWrite();
    for (int16_t j = 0; j < h; j++, y++) {
        for (int16_t i = 0; i < w; i++) {
            if (i & 7)
                byte <<= 1;
            else
                byte = bitmap[j * byteWidth + i / 8];
            writePixel(x + i, y, (byte & 0x80) ? color : bg);
        }
    }
    endWrite();
}

//============================================================================
// ROTATION AND VIRTUAL FUNCTIONS
//============================================================================

void Adafruit_GFX::setRotation(uint8_t x) {
    rotation = (x & 3);
    switch (rotation) {
    case 0:
    case 2:
        _width = WIDTH;
        _height = HEIGHT;
        break;
    case 1:
    case 3:
        _width = HEIGHT;
        _height = WIDTH;
        break;
    }
}

void Adafruit_GFX::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    drawLine(x0, y0, x1, y1, color);
}

void Adafruit_GFX::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    fillRect(x, y, w, h, color);
}