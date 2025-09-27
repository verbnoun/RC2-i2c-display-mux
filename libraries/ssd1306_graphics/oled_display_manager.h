//============================================================================
// OLED Display Manager
// High-level management of multiple SSD1306 displays with content system
//============================================================================

#ifndef OLED_DISPLAY_MANAGER_H
#define OLED_DISPLAY_MANAGER_H

#include "ssd1306_graphics.h"


//============================================================================
// CONFIGURATION
//============================================================================
#define MAX_DISPLAYS        8
#define MAX_CONTENT_LINES   4
#define MAX_LINE_LENGTH     22  // Approximately fits 6x8 font on 128px width

//============================================================================
// DISPLAY CONTENT STRUCTURE
//============================================================================
typedef struct {
    char lines[MAX_CONTENT_LINES][MAX_LINE_LENGTH];
    uint8_t line_count;
    bool needs_update;
    bool inverted;
    uint8_t contrast;
} display_content_t;

//============================================================================
// DISPLAY MANAGER CONTEXT
//============================================================================
typedef struct {
    ssd1306_t displays[MAX_DISPLAYS];
    display_content_t content[MAX_DISPLAYS];
    bool display_active[MAX_DISPLAYS];
    uint8_t active_count;
    i2c_inst_t* i2c_port;
    bool initialized;
} oled_manager_t;

//============================================================================
// CORE FUNCTIONS
//============================================================================

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize OLED display manager
 * @param manager Pointer to manager context
 * @param i2c_port I2C port to use (i2c0 or i2c1)
 * @return true if successful, false otherwise
 */
bool oled_manager_init(oled_manager_t* manager, i2c_inst_t* i2c_port);

/**
 * Scan for available displays and initialize them
 * @param manager Pointer to manager context
 * @return Number of displays found and initialized
 */
uint8_t oled_manager_scan_displays(oled_manager_t* manager);

/**
 * Update all displays that need refreshing
 * @param manager Pointer to manager context
 * @return Number of displays updated
 */
uint8_t oled_manager_update_all(oled_manager_t* manager);

/**
 * Shutdown manager and clean up resources
 * @param manager Pointer to manager context
 */
void oled_manager_shutdown(oled_manager_t* manager);

//============================================================================
// INDIVIDUAL DISPLAY FUNCTIONS
//============================================================================

/**
 * Set content for a specific display
 * @param manager Pointer to manager context
 * @param display_id Display index (0-7)
 * @param line Line number (0-3)
 * @param text Text to display (will be truncated if too long)
 * @return true if successful, false otherwise
 */
bool oled_manager_set_line(oled_manager_t* manager, uint8_t display_id, uint8_t line, const char* text);

/**
 * Clear all content on a display
 * @param manager Pointer to manager context
 * @param display_id Display index (0-7)
 * @return true if successful, false otherwise
 */
bool oled_manager_clear_display(oled_manager_t* manager, uint8_t display_id);

/**
 * Set display contrast
 * @param manager Pointer to manager context
 * @param display_id Display index (0-7)
 * @param contrast Contrast value (0-255)
 * @return true if successful, false otherwise
 */
bool oled_manager_set_contrast(oled_manager_t* manager, uint8_t display_id, uint8_t contrast);

/**
 * Invert display colors
 * @param manager Pointer to manager context
 * @param display_id Display index (0-7)
 * @param inverted true to invert, false for normal
 * @return true if successful, false otherwise
 */
bool oled_manager_set_inverted(oled_manager_t* manager, uint8_t display_id, bool inverted);

/**
 * Turn display on/off
 * @param manager Pointer to manager context
 * @param display_id Display index (0-7)
 * @param on true to turn on, false to turn off
 * @return true if successful, false otherwise
 */
bool oled_manager_set_display_on(oled_manager_t* manager, uint8_t display_id, bool on);

/**
 * Force update of a specific display
 * @param manager Pointer to manager context
 * @param display_id Display index (0-7)
 * @return true if successful, false otherwise
 */
bool oled_manager_update_display(oled_manager_t* manager, uint8_t display_id);

//============================================================================
// BATCH OPERATIONS
//============================================================================

/**
 * Set the same text line on all active displays
 * @param manager Pointer to manager context
 * @param line Line number (0-3)
 * @param text Text to display
 * @return Number of displays updated
 */
uint8_t oled_manager_set_all_lines(oled_manager_t* manager, uint8_t line, const char* text);

/**
 * Clear all displays
 * @param manager Pointer to manager context
 * @return Number of displays cleared
 */
uint8_t oled_manager_clear_all(oled_manager_t* manager);

/**
 * Set contrast on all displays
 * @param manager Pointer to manager context
 * @param contrast Contrast value (0-255)
 * @return Number of displays updated
 */
uint8_t oled_manager_set_all_contrast(oled_manager_t* manager, uint8_t contrast);

/**
 * Turn all displays on or off
 * @param manager Pointer to manager context
 * @param on true to turn on, false to turn off
 * @return Number of displays affected
 */
uint8_t oled_manager_set_all_on(oled_manager_t* manager, bool on);

//============================================================================
// UTILITY FUNCTIONS
//============================================================================

/**
 * Get number of active displays
 * @param manager Pointer to manager context
 * @return Number of active displays
 */
uint8_t oled_manager_get_active_count(oled_manager_t* manager);

/**
 * Check if a display is active
 * @param manager Pointer to manager context
 * @param display_id Display index (0-7)
 * @return true if display is active, false otherwise
 */
bool oled_manager_is_display_active(oled_manager_t* manager, uint8_t display_id);

/**
 * Get display content for debugging
 * @param manager Pointer to manager context
 * @param display_id Display index (0-7)
 * @return Pointer to content structure, NULL if invalid
 */
const display_content_t* oled_manager_get_content(oled_manager_t* manager, uint8_t display_id);

#ifdef __cplusplus
}
#endif

#endif // OLED_DISPLAY_MANAGER_H