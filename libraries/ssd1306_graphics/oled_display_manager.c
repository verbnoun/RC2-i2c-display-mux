//============================================================================
// OLED Display Manager Implementation
// High-level management of multiple SSD1306 displays with content system
//============================================================================

#include "oled_display_manager.h"
#include <string.h>
#include <stdio.h>

// Include the I2C MUX header for function definitions
// Note: This creates a circular dependency that needs to be resolved at link time
bool mux_init(void);
bool mux_select_channel(uint8_t channel);
bool mux_is_display_detected(uint8_t channel);
void mux_shutdown(void);

//============================================================================
// CORE FUNCTIONS
//============================================================================

bool oled_manager_init(oled_manager_t* manager, i2c_inst_t* i2c_port) {
    if (!manager || !i2c_port) return false;
    
    // Initialize manager context
    manager->i2c_port = i2c_port;
    manager->active_count = 0;
    manager->initialized = false;
    
    // Clear all display states
    for (int i = 0; i < MAX_DISPLAYS; i++) {
        manager->display_active[i] = false;
        manager->content[i].line_count = 0;
        manager->content[i].needs_update = false;
        manager->content[i].inverted = false;
        manager->content[i].contrast = 127;  // Default contrast
        
        // Clear content lines
        for (int j = 0; j < MAX_CONTENT_LINES; j++) {
            manager->content[i].lines[j][0] = '\0';
        }
    }
    
    // Initialize I2C multiplexer if not already done
    if (!mux_init()) {
        return false;
    }
    
    manager->initialized = true;
    return true;
}

uint8_t oled_manager_scan_displays(oled_manager_t* manager) {
    if (!manager || !manager->initialized) return 0;
    
    uint8_t found = 0;
    
    // Scan each possible multiplexer channel
    for (uint8_t channel = 0; channel < MAX_DISPLAYS; channel++) {
        // Check if multiplexer can select this channel
        if (!mux_select_channel(channel)) {
            continue;
        }
        
        // Check if display is detected on this channel
        if (!mux_is_display_detected(channel)) {
            continue;
        }
        
        // Initialize the SSD1306 display
        if (ssd1306_init(&manager->displays[channel], manager->i2c_port)) {
            manager->display_active[channel] = true;
            manager->content[channel].needs_update = true;
            found++;
            
            // Set default display name
            char display_name[MAX_LINE_LENGTH];
            snprintf(display_name, sizeof(display_name), "OLED%d", channel);
            oled_manager_set_line(manager, channel, 0, display_name);
            oled_manager_set_line(manager, channel, 1, "Ready");
        }
    }
    
    manager->active_count = found;
    return found;
}

uint8_t oled_manager_update_all(oled_manager_t* manager) {
    if (!manager || !manager->initialized) return 0;
    
    uint8_t updated = 0;
    
    for (uint8_t i = 0; i < MAX_DISPLAYS; i++) {
        if (manager->display_active[i] && manager->content[i].needs_update) {
            if (oled_manager_update_display(manager, i)) {
                updated++;
            }
        }
    }
    
    return updated;
}

void oled_manager_shutdown(oled_manager_t* manager) {
    if (!manager) return;
    
    // Clear all displays before shutdown
    oled_manager_clear_all(manager);
    oled_manager_update_all(manager);
    
    // Turn off all displays
    oled_manager_set_all_on(manager, false);
    
    // Reset state
    manager->active_count = 0;
    manager->initialized = false;
    
    for (int i = 0; i < MAX_DISPLAYS; i++) {
        manager->display_active[i] = false;
    }
    
    // Shutdown multiplexer
    mux_shutdown();
}

//============================================================================
// INDIVIDUAL DISPLAY FUNCTIONS
//============================================================================

bool oled_manager_set_line(oled_manager_t* manager, uint8_t display_id, uint8_t line, const char* text) {
    if (!manager || !manager->initialized || display_id >= MAX_DISPLAYS || 
        line >= MAX_CONTENT_LINES || !text) {
        return false;
    }
    
    if (!manager->display_active[display_id]) {
        return false;
    }
    
    // Copy text, ensuring null termination and length limit
    strncpy(manager->content[display_id].lines[line], text, MAX_LINE_LENGTH - 1);
    manager->content[display_id].lines[line][MAX_LINE_LENGTH - 1] = '\0';
    
    // Update line count
    if (line >= manager->content[display_id].line_count) {
        manager->content[display_id].line_count = line + 1;
    }
    
    // Mark for update
    manager->content[display_id].needs_update = true;
    
    return true;
}

bool oled_manager_clear_display(oled_manager_t* manager, uint8_t display_id) {
    if (!manager || !manager->initialized || display_id >= MAX_DISPLAYS) {
        return false;
    }
    
    if (!manager->display_active[display_id]) {
        return false;
    }
    
    // Clear all content lines
    for (int i = 0; i < MAX_CONTENT_LINES; i++) {
        manager->content[display_id].lines[i][0] = '\0';
    }
    
    manager->content[display_id].line_count = 0;
    manager->content[display_id].needs_update = true;
    
    return true;
}

bool oled_manager_set_contrast(oled_manager_t* manager, uint8_t display_id, uint8_t contrast) {
    if (!manager || !manager->initialized || display_id >= MAX_DISPLAYS) {
        return false;
    }
    
    if (!manager->display_active[display_id]) {
        return false;
    }
    
    manager->content[display_id].contrast = contrast;
    
    // Select the display channel
    if (!mux_select_channel(display_id)) {
        return false;
    }
    
    return ssd1306_set_contrast(&manager->displays[display_id], contrast);
}

bool oled_manager_set_inverted(oled_manager_t* manager, uint8_t display_id, bool inverted) {
    if (!manager || !manager->initialized || display_id >= MAX_DISPLAYS) {
        return false;
    }
    
    if (!manager->display_active[display_id]) {
        return false;
    }
    
    manager->content[display_id].inverted = inverted;
    
    // Select the display channel
    if (!mux_select_channel(display_id)) {
        return false;
    }
    
    return ssd1306_invert_display(&manager->displays[display_id], inverted);
}

bool oled_manager_set_display_on(oled_manager_t* manager, uint8_t display_id, bool on) {
    if (!manager || !manager->initialized || display_id >= MAX_DISPLAYS) {
        return false;
    }
    
    if (!manager->display_active[display_id]) {
        return false;
    }
    
    // Select the display channel
    if (!mux_select_channel(display_id)) {
        return false;
    }
    
    if (on) {
        return ssd1306_display_on(&manager->displays[display_id]);
    } else {
        return ssd1306_display_off(&manager->displays[display_id]);
    }
}

bool oled_manager_update_display(oled_manager_t* manager, uint8_t display_id) {
    if (!manager || !manager->initialized || display_id >= MAX_DISPLAYS) {
        return false;
    }
    
    if (!manager->display_active[display_id]) {
        return false;
    }
    
    // Select the display channel
    if (!mux_select_channel(display_id)) {
        return false;
    }
    
    ssd1306_t* display = &manager->displays[display_id];
    display_content_t* content = &manager->content[display_id];
    
    // Clear the display buffer
    ssd1306_clear(display);
    
    // Render each line of content
    uint8_t y_offset = 0;
    const uint8_t line_height = 16;  // 8 pixels + 8 pixels spacing
    
    for (uint8_t line = 0; line < content->line_count && line < MAX_CONTENT_LINES; line++) {
        if (content->lines[line][0] != '\0') {  // Non-empty line
            ssd1306_draw_string(display, 0, y_offset, content->lines[line], true);
            y_offset += line_height;
            
            // Stop if we've reached the bottom of the display
            if (y_offset >= SSD1306_HEIGHT - 8) {
                break;
            }
        }
    }
    
    // Update the physical display
    bool success = ssd1306_display(display);
    
    if (success) {
        content->needs_update = false;
    }
    
    return success;
}

//============================================================================
// BATCH OPERATIONS
//============================================================================

uint8_t oled_manager_set_all_lines(oled_manager_t* manager, uint8_t line, const char* text) {
    if (!manager || !manager->initialized) return 0;
    
    uint8_t updated = 0;
    
    for (uint8_t i = 0; i < MAX_DISPLAYS; i++) {
        if (manager->display_active[i]) {
            if (oled_manager_set_line(manager, i, line, text)) {
                updated++;
            }
        }
    }
    
    return updated;
}

uint8_t oled_manager_clear_all(oled_manager_t* manager) {
    if (!manager || !manager->initialized) return 0;
    
    uint8_t cleared = 0;
    
    for (uint8_t i = 0; i < MAX_DISPLAYS; i++) {
        if (manager->display_active[i]) {
            if (oled_manager_clear_display(manager, i)) {
                cleared++;
            }
        }
    }
    
    return cleared;
}

uint8_t oled_manager_set_all_contrast(oled_manager_t* manager, uint8_t contrast) {
    if (!manager || !manager->initialized) return 0;
    
    uint8_t updated = 0;
    
    for (uint8_t i = 0; i < MAX_DISPLAYS; i++) {
        if (manager->display_active[i]) {
            if (oled_manager_set_contrast(manager, i, contrast)) {
                updated++;
            }
        }
    }
    
    return updated;
}

uint8_t oled_manager_set_all_on(oled_manager_t* manager, bool on) {
    if (!manager || !manager->initialized) return 0;
    
    uint8_t updated = 0;
    
    for (uint8_t i = 0; i < MAX_DISPLAYS; i++) {
        if (manager->display_active[i]) {
            if (oled_manager_set_display_on(manager, i, on)) {
                updated++;
            }
        }
    }
    
    return updated;
}

//============================================================================
// UTILITY FUNCTIONS
//============================================================================

uint8_t oled_manager_get_active_count(oled_manager_t* manager) {
    if (!manager) return 0;
    return manager->active_count;
}

bool oled_manager_is_display_active(oled_manager_t* manager, uint8_t display_id) {
    if (!manager || display_id >= MAX_DISPLAYS) return false;
    return manager->display_active[display_id];
}

const display_content_t* oled_manager_get_content(oled_manager_t* manager, uint8_t display_id) {
    if (!manager || display_id >= MAX_DISPLAYS) return NULL;
    if (!manager->display_active[display_id]) return NULL;
    return &manager->content[display_id];
}