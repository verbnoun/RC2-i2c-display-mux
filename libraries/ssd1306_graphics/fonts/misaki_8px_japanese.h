//============================================================================
// Misaki Gothic 8px Japanese Font - Header
// Generated from authentic Misaki Gothic font (littlelimit.net/misaki.htm)
// License: Free software (very permissive)
// Characters: ASCII + Hiragana + Katakana + Japanese punctuation (249 total)
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
