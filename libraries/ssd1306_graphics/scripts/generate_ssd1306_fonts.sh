#!/bin/bash
#
# Japanese Font Generation Script for ssd1306_graphics Library
# Generates final .c/.h files from extracted bitmap data
#
# Usage: ./generate_ssd1306_fonts.sh
#

set -e

FONTS_DIR="/Users/jackson/Documents/Projects/Attach Part/Product/SoftwareC/libraries/ssd1306_graphics/fonts"

echo "=== Japanese Font Generation for ssd1306_graphics ==="
echo "Generating final font files from extracted bitmap data..."
echo ""

# Check if clean bitmap files exist
for font in "clean_misaki_8px.c" "clean_pixelmplus_10px.c" "clean_pixelmplus_12px.c"; do
    if [ ! -f "/tmp/$font" ]; then
        echo "Error: /tmp/$font not found. Run extract_font_bitmaps.sh first."
        exit 1
    fi
done

# Create fonts directory if it doesn't exist
mkdir -p "$FONTS_DIR"

echo "1. Generating Misaki 8px Japanese font..."

# Misaki 8px .c file
cat > "$FONTS_DIR/misaki_8px_japanese.c" << 'EOF'
//============================================================================
// Misaki Gothic 8px Japanese Font - Small Size
// Generated from authentic Misaki Gothic font (littlelimit.net/misaki.htm)
// License: Free software (very permissive)
// Characters: ASCII + Hiragana + Katakana + Japanese punctuation (244 total)
// Format: 8x8 pixels, 8 bytes per character
//============================================================================

#include "misaki_8px_japanese.h"

// Font bitmap data - 8x8 pixel characters in 2D array format
EOF

# Add the extracted bitmap data
cat /tmp/clean_misaki_8px.c >> "$FONTS_DIR/misaki_8px_japanese.c"

# Add the font structure
cat >> "$FONTS_DIR/misaki_8px_japanese.c" << 'EOF'

// Font structure compatible with ssd1306_graphics library
const ssd1306_font_t misaki_8px_japanese = {
    .width = 8,
    .height = 8,
    .first_char = 33,    // '!'
    .last_char = 126,    // '~' (ASCII range, Japanese chars need special mapping)
    .font_data = misaki_gothic8_bmp
};
EOF

# Misaki 8px .h file
cat > "$FONTS_DIR/misaki_8px_japanese.h" << 'EOF'
//============================================================================
// Misaki Gothic 8px Japanese Font - Header
// Generated from authentic Misaki Gothic font (littlelimit.net/misaki.htm)
// License: Free software (very permissive)
// Characters: ASCII + Hiragana + Katakana + Japanese punctuation (244 total)
// Format: 8x8 pixels, 8 bytes per character
//============================================================================

#ifndef MISAKI_8PX_JAPANESE_H
#define MISAKI_8PX_JAPANESE_H

#include <stdint.h>
#include "../ssd1306_graphics.h"

#ifdef __cplusplus
extern "C" {
#endif

// Font bitmap data - 8x8 pixel characters
extern const uint8_t misaki_gothic8_bmp[][8];

// Font structure for ssd1306_graphics library
extern const ssd1306_font_t misaki_8px_japanese;

#ifdef __cplusplus
}
#endif

#endif // MISAKI_8PX_JAPANESE_H
EOF

echo "✓ Misaki 8px font generated"

echo "2. Generating PixelMplus 10px Japanese font..."

# PixelMplus 10px .c file
cat > "$FONTS_DIR/pixelmplus_10px_japanese.c" << 'EOF'
//============================================================================
// PixelMplus 10px Japanese Font - Medium Size
// Generated from PixelMplus10-Regular font (github.com/itouhiro/PixelMplus)
// License: M+ FONTS LICENSE (MIT-compatible)
// Characters: ASCII + Hiragana + Katakana + Japanese punctuation (244 total)
// Format: 10x10 pixels, 13 bytes per character
//============================================================================

#include "pixelmplus_10px_japanese.h"

// Font bitmap data - 10x10 pixel characters in 2D array format
EOF

# Add the extracted bitmap data
cat /tmp/clean_pixelmplus_10px.c >> "$FONTS_DIR/pixelmplus_10px_japanese.c"

# Add the font structure
cat >> "$FONTS_DIR/pixelmplus_10px_japanese.c" << 'EOF'

// Font structure compatible with ssd1306_graphics library
const ssd1306_font_t pixelmplus_10px_japanese = {
    .width = 10,
    .height = 10,
    .first_char = 33,    // '!'
    .last_char = 126,    // '~' (ASCII range, Japanese chars need special mapping)
    .font_data = pixelmplus10_regular10_bmp
};
EOF

# PixelMplus 10px .h file
cat > "$FONTS_DIR/pixelmplus_10px_japanese.h" << 'EOF'
//============================================================================
// PixelMplus 10px Japanese Font - Header
// Generated from PixelMplus10-Regular font (github.com/itouhiro/PixelMplus)
// License: M+ FONTS LICENSE (MIT-compatible)
// Characters: ASCII + Hiragana + Katakana + Japanese punctuation (244 total)
// Format: 10x10 pixels, 13 bytes per character
//============================================================================

#ifndef PIXELMPLUS_10PX_JAPANESE_H
#define PIXELMPLUS_10PX_JAPANESE_H

#include <stdint.h>
#include "../ssd1306_graphics.h"

#ifdef __cplusplus
extern "C" {
#endif

// Font bitmap data - 10x10 pixel characters
extern const uint8_t pixelmplus10_regular10_bmp[][13];

// Font structure for ssd1306_graphics library
extern const ssd1306_font_t pixelmplus_10px_japanese;

#ifdef __cplusplus
}
#endif

#endif // PIXELMPLUS_10PX_JAPANESE_H
EOF

echo "✓ PixelMplus 10px font generated"

echo "3. Generating PixelMplus 12px Japanese font..."

# PixelMplus 12px .c file
cat > "$FONTS_DIR/pixelmplus_12px_japanese.c" << 'EOF'
//============================================================================
// PixelMplus 12px Japanese Font - Large Size
// Generated from PixelMplus12-Regular font (github.com/itouhiro/PixelMplus)
// License: M+ FONTS LICENSE (MIT-compatible)
// Characters: ASCII + Hiragana + Katakana + Japanese punctuation (244 total)
// Format: 12x12 pixels, 18 bytes per character
//============================================================================

#include "pixelmplus_12px_japanese.h"

// Font bitmap data - 12x12 pixel characters in 2D array format
EOF

# Add the extracted bitmap data
cat /tmp/clean_pixelmplus_12px.c >> "$FONTS_DIR/pixelmplus_12px_japanese.c"

# Add the font structure
cat >> "$FONTS_DIR/pixelmplus_12px_japanese.c" << 'EOF'

// Font structure compatible with ssd1306_graphics library
const ssd1306_font_t pixelmplus_12px_japanese = {
    .width = 12,
    .height = 12,
    .first_char = 33,    // '!'
    .last_char = 126,    // '~' (ASCII range, Japanese chars need special mapping)
    .font_data = pixelmplus12_regular12_bmp
};
EOF

# PixelMplus 12px .h file
cat > "$FONTS_DIR/pixelmplus_12px_japanese.h" << 'EOF'
//============================================================================
// PixelMplus 12px Japanese Font - Header
// Generated from PixelMplus12-Regular font (github.com/itouhiro/PixelMplus)
// License: M+ FONTS LICENSE (MIT-compatible)
// Characters: ASCII + Hiragana + Katakana + Japanese punctuation (244 total)
// Format: 12x12 pixels, 18 bytes per character
//============================================================================

#ifndef PIXELMPLUS_12PX_JAPANESE_H
#define PIXELMPLUS_12PX_JAPANESE_H

#include <stdint.h>
#include "../ssd1306_graphics.h"

#ifdef __cplusplus
extern "C" {
#endif

// Font bitmap data - 12x12 pixel characters
extern const uint8_t pixelmplus12_regular12_bmp[][18];

// Font structure for ssd1306_graphics library
extern const ssd1306_font_t pixelmplus_12px_japanese;

#ifdef __cplusplus
}
#endif

#endif // PIXELMPLUS_12PX_JAPANESE_H
EOF

echo "✓ PixelMplus 12px font generated"
echo ""

echo "=== Generation Summary ==="
echo "Generated fonts in: $FONTS_DIR"
echo "✓ misaki_8px_japanese.c/.h (8x8, 8 bytes/char, 244 chars)"
echo "✓ pixelmplus_10px_japanese.c/.h (10x10, 13 bytes/char, 244 chars)"
echo "✓ pixelmplus_12px_japanese.c/.h (12x12, 18 bytes/char, 244 chars)"
echo ""
echo "All fonts include:"
echo "- ASCII characters (!, \", #, ..., ~)"
echo "- Hiragana characters (あ, い, う, ...)"
echo "- Katakana characters (ア, イ, ウ, ...)"
echo "- Japanese punctuation (、。「」【】・)"
echo ""
echo "Ready for integration with ssd1306_graphics library!"
echo "Next steps:"
echo "1. Update CMakeLists.txt to include new font .c files"
echo "2. Update character mapping for Japanese characters"
echo "3. Test rendering"