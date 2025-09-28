//============================================================================
// Japanese Character Mapping System for SSD1306 Graphics
// Maps UTF-8 Japanese characters to font array indices
//============================================================================

#ifndef JAPANESE_CHAR_MAP_H
#define JAPANESE_CHAR_MAP_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//============================================================================
// CHARACTER MAPPING FUNCTIONS
//============================================================================

/**
 * Get font array index for a UTF-8 character
 * @param utf8_char Pointer to UTF-8 character bytes
 * @param bytes_consumed Pointer to store number of bytes consumed
 * @return Font array index, or 0xFFFF if character not found
 */
uint16_t japanese_char_to_index(const uint8_t* utf8_char, uint8_t* bytes_consumed);

/**
 * Get character width in pixels for UTF-8 character
 * @param utf8_char Pointer to UTF-8 character bytes
 * @param font Current font structure
 * @return Character width in pixels
 */
uint8_t japanese_char_width(const uint8_t* utf8_char, const void* font);

/**
 * Check if UTF-8 character is supported in current font
 * @param utf8_char Pointer to UTF-8 character bytes
 * @return true if character is supported, false otherwise
 */
bool japanese_char_supported(const uint8_t* utf8_char);

#ifdef __cplusplus
}
#endif

#endif // JAPANESE_CHAR_MAP_H