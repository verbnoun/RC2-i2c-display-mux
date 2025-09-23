#ifndef ACTIVITY_LED_H
#define ACTIVITY_LED_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "console_logger.h"

// LED activity modes
typedef enum {
    LED_MODE_HEARTBEAT_ONLY,    // Heartbeat during idle, off during activity
    LED_MODE_ACTIVITY_FLASH,    // Heartbeat during idle, flash on activity
    LED_MODE_ALWAYS_ON,         // LED always on
    LED_MODE_ALWAYS_OFF         // LED always off
} led_mode_t;

// Activity LED configuration
typedef struct {
    uint led_pin;                    // GPIO pin for LED
    uint32_t heartbeat_interval_ms;  // Heartbeat blink interval (default: 1000ms)
    uint32_t activity_flash_ms;      // Activity flash duration (default: 100ms)
    uint32_t activity_timeout_ms;    // Activity timeout period (default: 2000ms)
    led_mode_t mode;                 // LED operating mode
} activity_led_config_t;

// Default configuration values
#define ACTIVITY_LED_DEFAULT_HEARTBEAT_INTERVAL_MS 1000
#define ACTIVITY_LED_DEFAULT_ACTIVITY_FLASH_MS 200      // Increased for better visibility
#define ACTIVITY_LED_DEFAULT_ACTIVITY_TIMEOUT_MS 1000   // Reduced for faster return to heartbeat

class ActivityLED {
public:
    /**
     * Initialize the activity LED with configuration
     * @param config LED configuration settings
     * @return true if initialization successful
     */
    bool init(const activity_led_config_t& config);
    
    /**
     * Update LED state - call regularly from main loop
     * Handles heartbeat and activity flash timing
     */
    void update();
    
    /**
     * Report activity to trigger LED flash
     * Call whenever any input activity occurs (pot changes, button presses, etc.)
     */
    void reportActivity();
    
    /**
     * Set LED operating mode
     * @param mode New LED mode
     */
    void setMode(led_mode_t mode);
    
    /**
     * Get current LED mode
     * @return Current operating mode
     */
    led_mode_t getMode() const { return config_.mode; }
    
    /**
     * Get current LED state
     * @return true if LED is currently on
     */
    bool getLEDState() const { return led_state_; }
    
    /**
     * Force LED state (overrides automatic control temporarily)
     * @param state LED state to set
     * @param duration_ms Duration to hold state (0 = permanent until next update)
     */
    void forceLEDState(bool state, uint32_t duration_ms = 0);
    
    /**
     * Check if there has been recent activity
     * @return true if activity occurred within timeout period
     */
    bool hasRecentActivity() const;
    
    /**
     * Get time since last activity
     * @return milliseconds since last reported activity
     */
    uint32_t getTimeSinceLastActivity() const;

private:
    activity_led_config_t config_;
    
    // LED state tracking
    bool led_state_;
    uint32_t last_led_toggle_time_;
    uint32_t last_activity_time_;
    uint32_t force_state_end_time_;
    bool force_state_active_;
    
    /**
     * Set physical LED state
     * @param state LED state to set
     */
    void setLEDPhysical(bool state);
    
    /**
     * Update heartbeat LED pattern
     */
    void updateHeartbeat();
    
    /**
     * Update activity flash pattern
     */
    void updateActivityFlash();
};

// Convenience macros for activity LED logging
#define LOG_LED_DEBUG(...)   LOG_DEBUG(TAG_HW, __VA_ARGS__)
#define LOG_LED_INFO(...)    LOG_INFO(TAG_HW, __VA_ARGS__)
#define LOG_LED_WARN(...)    LOG_WARN(TAG_HW, __VA_ARGS__)
#define LOG_LED_ERROR(...)   LOG_ERROR(TAG_HW, __VA_ARGS__)

#endif // ACTIVITY_LED_H