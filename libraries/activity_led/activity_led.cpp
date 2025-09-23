#include "activity_led.h"

bool ActivityLED::init(const activity_led_config_t& config) {
    LOG_LED_INFO("Initializing activity LED...");
    
    // Store configuration
    config_ = config;
    
    // Initialize GPIO pin
    gpio_init(config_.led_pin);
    gpio_set_dir(config_.led_pin, GPIO_OUT);
    
    // Initialize state
    led_state_ = false;
    last_led_toggle_time_ = to_ms_since_boot(get_absolute_time());
    last_activity_time_ = 0;
    force_state_end_time_ = 0;
    force_state_active_ = false;
    
    // Set initial LED state
    setLEDPhysical(led_state_);
    
    LOG_LED_INFO("Activity LED initialized on GP%d, mode: %d", 
                 config_.led_pin, config_.mode);
    
    return true;
}

void ActivityLED::update() {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Check if force state is active and expired
    if (force_state_active_ && current_time >= force_state_end_time_) {
        force_state_active_ = false;
        LOG_LED_DEBUG("Force state expired");
    }
    
    // If force state is active, don't update automatically
    if (force_state_active_) {
        return;
    }
    
    switch (config_.mode) {
        case LED_MODE_HEARTBEAT_ONLY:
            updateHeartbeat();
            break;
            
        case LED_MODE_ACTIVITY_FLASH:
            updateActivityFlash();
            break;
            
        case LED_MODE_ALWAYS_ON:
            if (!led_state_) {
                setLEDPhysical(true);
            }
            break;
            
        case LED_MODE_ALWAYS_OFF:
            if (led_state_) {
                setLEDPhysical(false);
            }
            break;
    }
}

void ActivityLED::reportActivity() {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    last_activity_time_ = current_time;
    
    LOG_LED_DEBUG("Activity reported");
    
    // If in activity flash mode, trigger flash (but respect minimum flash interval)
    if (config_.mode == LED_MODE_ACTIVITY_FLASH) {
        // Only turn on if we've been off for at least half the flash duration
        // This creates a max blink rate even during continuous activity
        if (!led_state_ && (current_time - last_led_toggle_time_ >= config_.activity_flash_ms / 2)) {
            setLEDPhysical(true);
            last_led_toggle_time_ = current_time;
        }
    }
}

void ActivityLED::setMode(led_mode_t mode) {
    LOG_LED_INFO("LED mode changed from %d to %d", config_.mode, mode);
    config_.mode = mode;
    
    // Reset state for new mode
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    last_led_toggle_time_ = current_time;
}

void ActivityLED::forceLEDState(bool state, uint32_t duration_ms) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    setLEDPhysical(state);
    
    if (duration_ms > 0) {
        force_state_active_ = true;
        force_state_end_time_ = current_time + duration_ms;
        LOG_LED_DEBUG("LED forced to %s for %dms", state ? "ON" : "OFF", duration_ms);
    } else {
        force_state_active_ = false;
        LOG_LED_DEBUG("LED forced to %s permanently", state ? "ON" : "OFF");
    }
}

bool ActivityLED::hasRecentActivity() const {
    if (last_activity_time_ == 0) {
        return false;
    }
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    return (current_time - last_activity_time_) < config_.activity_timeout_ms;
}

uint32_t ActivityLED::getTimeSinceLastActivity() const {
    if (last_activity_time_ == 0) {
        return UINT32_MAX;
    }
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    return current_time - last_activity_time_;
}

void ActivityLED::setLEDPhysical(bool state) {
    if (led_state_ != state) {
        led_state_ = state;
        gpio_put(config_.led_pin, state);
        LOG_LED_DEBUG("LED %s", state ? "ON" : "OFF");
    }
}

void ActivityLED::updateHeartbeat() {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Simple heartbeat - toggle every heartbeat_interval_ms
    if (current_time - last_led_toggle_time_ >= config_.heartbeat_interval_ms) {
        setLEDPhysical(!led_state_);
        last_led_toggle_time_ = current_time;
    }
}

void ActivityLED::updateActivityFlash() {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    bool recent_activity = hasRecentActivity();
    
    if (recent_activity) {
        // During activity period: flash on activity, then off after flash duration
        if (led_state_ && (current_time - last_led_toggle_time_ >= config_.activity_flash_ms)) {
            setLEDPhysical(false);
            LOG_LED_DEBUG("Activity flash OFF");
        }
    } else {
        // No recent activity: heartbeat mode
        if (current_time - last_led_toggle_time_ >= config_.heartbeat_interval_ms) {
            setLEDPhysical(!led_state_);
            last_led_toggle_time_ = current_time;
            LOG_LED_DEBUG("Heartbeat LED %s", led_state_ ? "ON" : "OFF");
        }
    }
}