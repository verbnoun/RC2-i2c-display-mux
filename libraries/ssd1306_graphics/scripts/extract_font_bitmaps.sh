#!/bin/bash
#
# Japanese Font Bitmap Extraction Script
# Extracts clean bitmap arrays from font2c generated files
# 
# Usage: ./extract_font_bitmaps.sh
#

set -e

echo "=== Japanese Font Bitmap Extraction ==="
echo "Extracting bitmap arrays from font2c output..."

# Check if export directory exists
if [ ! -d "/tmp/export" ]; then
    echo "Error: /tmp/export directory not found. Run font2c first."
    exit 1
fi

# Extract bitmap data from each font
echo "Extracting Misaki 8px bitmap data..."
if [ -f "/tmp/export/misaki_8px.c" ]; then
    sed -n '/const uint8_t.*_bmp\[\]\[.*\] = {/,/^};$/p' /tmp/export/misaki_8px.c > /tmp/clean_misaki_8px.c
    echo "✓ Misaki 8px: $(wc -l < /tmp/clean_misaki_8px.c) lines extracted"
else
    echo "✗ Error: /tmp/export/misaki_8px.c not found"
    exit 1
fi

echo "Extracting PixelMplus 10px bitmap data..."
if [ -f "/tmp/export/pixelmplus_10px.c" ]; then
    sed -n '/const uint8_t.*_bmp\[\]\[.*\] = {/,/^};$/p' /tmp/export/pixelmplus_10px.c > /tmp/clean_pixelmplus_10px.c
    echo "✓ PixelMplus 10px: $(wc -l < /tmp/clean_pixelmplus_10px.c) lines extracted"
else
    echo "✗ Error: /tmp/export/pixelmplus_10px.c not found"
    exit 1
fi

echo "Extracting PixelMplus 12px bitmap data..."
if [ -f "/tmp/export/pixelmplus_12px.c" ]; then
    sed -n '/const uint8_t.*_bmp\[\]\[.*\] = {/,/^};$/p' /tmp/export/pixelmplus_12px.c > /tmp/clean_pixelmplus_12px.c
    echo "✓ PixelMplus 12px: $(wc -l < /tmp/clean_pixelmplus_12px.c) lines extracted"
else
    echo "✗ Error: /tmp/export/pixelmplus_12px.c not found"
    exit 1
fi

echo ""
echo "=== Extraction Summary ==="
echo "Misaki 8px:      $(grep -c '{' /tmp/clean_misaki_8px.c) characters"
echo "PixelMplus 10px: $(grep -c '{' /tmp/clean_pixelmplus_10px.c) characters"  
echo "PixelMplus 12px: $(grep -c '{' /tmp/clean_pixelmplus_12px.c) characters"
echo ""
echo "Clean bitmap files ready in /tmp/"
echo "✓ /tmp/clean_misaki_8px.c"
echo "✓ /tmp/clean_pixelmplus_10px.c" 
echo "✓ /tmp/clean_pixelmplus_12px.c"
echo ""
echo "Extraction complete!"