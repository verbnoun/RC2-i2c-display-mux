#!/bin/bash
#
# Master Japanese Font Rebuild Script
# Complete pipeline from source fonts to ssd1306_graphics integration
#
# Usage: ./rebuild_japanese_fonts.sh
#
# This script runs the complete font generation pipeline:
# 1. Generate fonts with font2c
# 2. Extract bitmap data
# 3. Create ssd1306_graphics font files
#

set -e

echo "================================================="
echo "Japanese Font Rebuild Pipeline - Master Script"
echo "================================================="
echo ""

# Check if font2c directory exists
if [ ! -d "/tmp/font2c" ]; then
    echo "Error: /tmp/font2c directory not found."
    echo "Please ensure font2c is available in /tmp/font2c/"
    exit 1
fi

# Check if source fonts exist
if [ ! -f "/tmp/misaki_gothic.ttf" ]; then
    echo "Error: /tmp/misaki_gothic.ttf not found."
    echo "Please ensure source fonts are available in /tmp/"
    exit 1
fi

if [ ! -d "/tmp/PixelMplus" ]; then
    echo "Error: /tmp/PixelMplus directory not found."
    echo "Please ensure PixelMplus fonts are available in /tmp/PixelMplus/"
    exit 1
fi

echo "Step 1: Generating fonts with font2c..."
echo "======================================="

cd /tmp/font2c

echo "→ Generating Misaki 8px font..."
python3 font2c.py /tmp/misaki_8px_config.ini

echo "→ Generating PixelMplus 10px font..."  
python3 font2c.py /tmp/pixelmplus_10px_config.ini

echo "→ Generating PixelMplus 12px font..."
python3 font2c.py /tmp/pixelmplus_12px_config.ini

echo "✓ font2c generation complete"
echo ""

echo "Step 2: Extracting bitmap data..."
echo "=================================="

/tmp/extract_font_bitmaps.sh

echo ""

echo "Step 3: Generating ssd1306_graphics font files..."
echo "=================================================="

/tmp/generate_ssd1306_fonts.sh

echo ""

echo "================================================="
echo "Japanese Font Rebuild Complete!"
echo "================================================="
echo ""
echo "Generated 3 fonts with complete Japanese character support:"
echo "• Misaki 8px (小):      244 chars, 8 bytes each"
echo "• PixelMplus 10px (中): 244 chars, 13 bytes each"  
echo "• PixelMplus 12px (大): 244 chars, 18 bytes each"
echo ""
echo "Character set includes:"
echo "• ASCII: !\"#$%&'()*+,-./...~"
echo "• Japanese punctuation: 、。「」【】・"
echo "• Hiragana: あいうえお..."
echo "• Katakana: アイウエオ..."
echo ""
echo "Files generated:"
echo "• /Users/.../ssd1306_graphics/fonts/misaki_8px_japanese.c/.h"
echo "• /Users/.../ssd1306_graphics/fonts/pixelmplus_10px_japanese.c/.h"
echo "• /Users/.../ssd1306_graphics/fonts/pixelmplus_12px_japanese.c/.h"
echo ""
echo "Next steps:"
echo "1. Update CMakeLists.txt to include new font .c files"
echo "2. Update character mapping for Japanese characters"  
echo "3. Test rendering with actual hardware"
echo ""
echo "🎌 Ready for Japanese text display! 🎌"