#include "pot_scanner.h"
#include <algorithm>
#include <cmath>

PotScanner::PotScanner() : current_channel_(0), scanning_active_(false), adc_channel_(0), 
                           current_external_channel_(0), total_scans_(0), last_rate_check_time_(0), 
                           scans_since_rate_check_(0) {
    // Constructor - initialize member variables
}

PotScanner::~PotScanner() {
    // Destructor - cleanup if needed
}

bool PotScanner::init(const pot_scanner_config_t& config) {
    LOG(TAG_POT, "Initializing pot scanner...");
    
    // Store configuration
    config_ = config;
    
    // Validate ADC pin
    if (config_.adc_pin < 26 || config_.adc_pin > 29) {
        LOG(TAG_POT, "ERROR: Invalid ADC pin %d - must be GP26-GP29", config_.adc_pin);
        return false;
    }
    
    // Initialize ADC
    adc_init();
    
    // Configure ADC input pin
    adc_gpio_init(config_.adc_pin);
    adc_channel_ = config_.adc_pin - 26;  // GP26=ADC0, GP27=ADC1, etc.
    adc_select_input(adc_channel_);
    current_external_channel_ = 0;
    
    LOG(TAG_POT, "ADC initialized on GP%d (ADC%d) - external channel control", 
                 config_.adc_pin, adc_channel_);
    
    // Initialize all channel states
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    for (uint8_t i = 0; i < POT_SCANNER_MAX_CHANNELS; i++) {
        pot_state_t& pot = channels_[i];
        
        // Reset all values
        pot.raw_value = 0;
        pot.current_value = 0;
        pot.previous_value = 0;
        pot.last_reported_value = 0;
        pot.mapped_value = 0;
        pot.ema_value = 0.0f;
        
        // Reset flags and timestamps
        pot.has_changed = false;
        pot.last_change_time = 0;
        pot.last_report_time = 0;
        pot.last_scan_time = 0;
        pot.last_movement_time = current_time;
        pot.is_quiet = true;
        
        // Reset tracking
        pot.last_direction = 0;
        pot.direction_consistency = 0;
        pot.stability_count = 0;
        pot.min_value = POT_SCANNER_ADC_RESOLUTION;
        pot.max_value = 0;
        pot.is_active = true;  // All channels active by default
    }
    
    // Initialize scanner state
    current_channel_ = 0;
    scanning_active_ = false;
    total_scans_ = 0;
    last_rate_check_time_ = current_time;
    scans_since_rate_check_ = 0;
    
    LOG(TAG_POT, "Pot scanner initialized - %d channels, EMA alpha=%.2f, thresholds: move=%d quiet=%d", 
                 POT_SCANNER_MAX_CHANNELS, config_.ema_alpha, 
                 config_.movement_threshold, config_.quiet_threshold);
    
    return true;
}

void PotScanner::startScanning() {
    if (!scanning_active_) {
        scanning_active_ = true;
        current_channel_ = 0;
        LOG(TAG_POT, "Pot scanning started");
    }
}

void PotScanner::stopScanning() {
    if (scanning_active_) {
        scanning_active_ = false;
        LOG(TAG_POT, "Pot scanning stopped");
    }
}

void PotScanner::update() {
    if (!scanning_active_) {
        return;
    }
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Check if it's time to scan the current channel
    if (current_time - channels_[current_channel_].last_scan_time < config_.scan_interval_ms) {
        return;
    }
    
    // Skip inactive channels
    if (!channels_[current_channel_].is_active) {
        // Move to next channel
        current_channel_ = (current_channel_ + 1) % POT_SCANNER_MAX_CHANNELS;
        return;
    }
    
    // Update the current channel
    updatePot(current_channel_);
    
    // Update timing and metrics
    channels_[current_channel_].last_scan_time = current_time;
    total_scans_++;
    scans_since_rate_check_++;
    
    // Move to next channel
    current_channel_ = (current_channel_ + 1) % POT_SCANNER_MAX_CHANNELS;
}

void PotScanner::updatePot(uint8_t channel) {
    // External channel selection is handled by caller (main.cpp)
    // Just read the current ADC value - channel should already be selected
    uint16_t raw_value = readCurrentADC();
    pot_state_t& pot = channels_[channel];
    pot.raw_value = raw_value;
    
    // Update EMA filter
    updateEMA(channel, raw_value);
    
    // Use filtered value for logic
    uint16_t filtered_value = (uint16_t)(pot.ema_value + 0.5f); // Round to nearest int
    
    // Update min/max tracking with filtered value
    pot.min_value = std::min(pot.min_value, filtered_value);
    pot.max_value = std::max(pot.max_value, filtered_value);
    
    // Update quiescence detection
    updateQuiescence(channel);
    
    // If pot is quiet, don't report any changes
    if (pot.is_quiet) {
        pot.current_value = filtered_value;
        pot.stability_count = 0;
        return;
    }
    
    // Calculate change from last reported value
    uint16_t diff_from_reported = (filtered_value > pot.last_reported_value) ? 
                                   (filtered_value - pot.last_reported_value) : 
                                   (pot.last_reported_value - filtered_value);
    
    // Determine current direction
    int8_t current_direction = 0;
    uint16_t movement_threshold_3rd = config_.movement_threshold / 3;
    if (filtered_value > pot.current_value + movement_threshold_3rd) {
        current_direction = 1;
        pot.direction_consistency = std::min((int)pot.direction_consistency + 1, 5);
    } else if (filtered_value < pot.current_value - movement_threshold_3rd) {
        current_direction = -1;
        pot.direction_consistency = std::min((int)pot.direction_consistency + 1, 5);
    } else {
        pot.direction_consistency = 0;
    }
    
    // Check if we should report a change
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    bool time_ok = (current_time - pot.last_report_time) >= config_.min_report_interval_ms;
    
    if (diff_from_reported >= config_.movement_threshold && time_ok && pot.direction_consistency >= 1) {
        pot.stability_count++;
        
        if (pot.stability_count >= config_.stability_required) {
            // Report the change
            pot.previous_value = pot.current_value;
            pot.current_value = filtered_value;
            pot.last_reported_value = filtered_value;
            
            // Map to MIDI range and check for actual change
            uint8_t new_mapped = mapValue(filtered_value);
            if (new_mapped != pot.mapped_value) {
                pot.mapped_value = new_mapped;
                pot.has_changed = true;
                pot.last_change_time = current_time;
                pot.last_report_time = current_time;
                pot.last_movement_time = current_time;
                
                LOG(TAG_POT, "Ch%d: raw=%d, ema=%.1f, mapped=%d", 
                    channel, raw_value, pot.ema_value, new_mapped);
            }
            pot.stability_count = 0;
        }
    } else {
        // Only update current value if it's a significant change from current, not just reported
        uint16_t diff_from_current = (filtered_value > pot.current_value) ?
                                     (filtered_value - pot.current_value) :
                                     (pot.current_value - filtered_value);
        
        if (diff_from_current >= config_.movement_threshold) {
            pot.previous_value = pot.current_value;
            pot.current_value = filtered_value;
        }
        
        // Reset stability if no significant change
        if (diff_from_reported < config_.movement_threshold) {
            pot.stability_count = 0;
        }
    }
    
    // Update direction tracking
    pot.last_direction = current_direction;
}

void PotScanner::updateEMA(uint8_t channel, uint16_t raw_value) {
    pot_state_t& pot = channels_[channel];
    
    // Initialize EMA on first reading
    if (pot.ema_value == 0.0f) {
        pot.ema_value = (float)raw_value;
        pot.last_reported_value = raw_value;
        pot.current_value = raw_value;
    } else {
        // Apply exponential moving average
        pot.ema_value = (config_.ema_alpha * raw_value) + ((1.0f - config_.ema_alpha) * pot.ema_value);
    }
}

void PotScanner::updateQuiescence(uint8_t channel) {
    pot_state_t& pot = channels_[channel];
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Check if there's been significant movement recently
    uint16_t filtered_value = (uint16_t)(pot.ema_value + 0.5f);
    uint16_t diff_from_last = (filtered_value > pot.current_value) ?
                             (filtered_value - pot.current_value) :
                             (pot.current_value - filtered_value);
    
    if (diff_from_last >= config_.quiet_threshold) {
        // Significant movement detected
        pot.last_movement_time = current_time;
        pot.is_quiet = false;
    } else if (current_time - pot.last_movement_time >= config_.quiet_time_ms) {
        // No significant movement for quiet_time_, consider quiet
        pot.is_quiet = true;
    }
}

void PotScanner::setChannelEnabled(uint8_t channel, bool enabled) {
    if (!isValidChannel(channel)) {
        LOG(TAG_POT, "WARN: Invalid channel %d for enable/disable", channel);
        return;
    }
    
    channels_[channel].is_active = enabled;
    LOG(TAG_POT, "Channel %d %s", channel, enabled ? "enabled" : "disabled");
}

uint8_t PotScanner::getValue(uint8_t channel) const {
    if (!isValidChannel(channel)) {
        return 0;
    }
    return channels_[channel].mapped_value;
}

uint16_t PotScanner::getRawValue(uint8_t channel) const {
    if (!isValidChannel(channel)) {
        return 0;
    }
    return channels_[channel].raw_value;
}

bool PotScanner::hasValueChanged(uint8_t channel) {
    if (!isValidChannel(channel)) {
        return false;
    }
    
    bool changed = channels_[channel].has_changed;
    channels_[channel].has_changed = false;  // Clear flag after reading
    return changed;
}

void PotScanner::printStatus() const {
    LOG(TAG_POT, "=== Pot Scanner Status ===");
    LOG(TAG_POT, "Scanning: %s, Total scans: %lu, Rate: %.1f Hz", 
                 scanning_active_ ? "active" : "stopped", 
                 total_scans_, getCurrentScanRate());
    LOG(TAG_POT, "EMA alpha: %.2f, Movement: %d, Quiet: %d, Quiet time: %dms", 
                 config_.ema_alpha, config_.movement_threshold, 
                 config_.quiet_threshold, config_.quiet_time_ms);
    
    // Show active channels with current values
    for (uint8_t i = 0; i < POT_SCANNER_MAX_CHANNELS; i++) {
        if (channels_[i].is_active) {
            const pot_state_t& pot = channels_[i];
            LOG(TAG_POT, "Ch%2d: raw=%4d, ema=%6.1f, mapped=%3d, %s", 
                         i, pot.raw_value, pot.ema_value, pot.mapped_value,
                         pot.is_quiet ? "quiet" : "ACTIVE");
        }
    }
}

void PotScanner::printChannelDiagnostics(uint8_t channel) const {
    if (!isValidChannel(channel)) {
        LOG(TAG_POT, "ERROR: Invalid channel %d for diagnostics", channel);
        return;
    }
    
    const pot_state_t& pot = channels_[channel];
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    LOG(TAG_POT, "=== Channel %d Diagnostics ===", channel);
    LOG(TAG_POT, "Active: %s", pot.is_active ? "yes" : "no");
    LOG(TAG_POT, "Raw: %d, EMA: %.1f, Current: %d, Mapped: %d", 
                 pot.raw_value, pot.ema_value, pot.current_value, pot.mapped_value);
    LOG(TAG_POT, "Quiet: %s, Last movement: %lu ms ago", 
                 pot.is_quiet ? "YES" : "NO",
                 current_time - pot.last_movement_time);
    LOG(TAG_POT, "Direction: %d, Consistency: %d, Stability: %d/%d", 
                 pot.last_direction, pot.direction_consistency, 
                 pot.stability_count, config_.stability_required);
    LOG(TAG_POT, "Min: %d, Max: %d, Range: %d", 
                 pot.min_value, pot.max_value, pot.max_value - pot.min_value);
}

float PotScanner::getCurrentScanRate() const {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    uint32_t time_elapsed = current_time - last_rate_check_time_;
    
    if (time_elapsed == 0) {
        return 0.0f;
    }
    
    return (scans_since_rate_check_ * 1000.0f) / time_elapsed;
}

void PotScanner::clearChangedFlags() {
    for (uint8_t i = 0; i < POT_SCANNER_MAX_CHANNELS; i++) {
        channels_[i].has_changed = false;
    }
}

bool PotScanner::anyPotChanged() const {
    for (uint8_t i = 0; i < POT_SCANNER_MAX_CHANNELS; i++) {
        if (channels_[i].has_changed) {
            return true;
        }
    }
    return false;
}

uint8_t PotScanner::getActivePotCount() const {
    uint8_t count = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    for (uint8_t i = 0; i < POT_SCANNER_MAX_CHANNELS; i++) {
        // Consider a pot "active" if it changed in the last 5 seconds
        if (channels_[i].last_change_time > 0 && 
            (current_time - channels_[i].last_change_time) < 5000) {
            count++;
        }
    }
    return count;
}

void PotScanner::calibrate() {
    LOG(TAG_POT, "Calibrating potentiometers...");
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Take several readings to establish baseline
    for (int sample = 0; sample < 10; sample++) {
        for (uint8_t channel = 0; channel < POT_SCANNER_MAX_CHANNELS; channel++) {
            if (!channels_[channel].is_active) continue;
            
            // External channel selection must be handled by caller for calibration
            // For now, just read current channel - caller should iterate through channels
            uint16_t value = readCurrentADC();
            
            if (sample == 0) {
                pot_state_t& pot = channels_[channel];
                pot.current_value = value;
                pot.raw_value = value;
                pot.previous_value = value;
                pot.last_reported_value = value;
                pot.ema_value = (float)value;
                pot.last_movement_time = current_time;
                pot.is_quiet = true;
                pot.last_direction = 0;
                pot.direction_consistency = 0;
                pot.stability_count = 0;
                pot.has_changed = false;
            }
        }
        sleep_ms(10);
    }
    
    LOG(TAG_POT, "Calibration complete - EMA filters reset");
}

void PotScanner::selectExternalChannel(uint8_t channel) {
    // Track which channel is currently selected externally
    // This is for reference only - actual mux control is handled by caller
    current_external_channel_ = channel;
}

uint16_t PotScanner::readCurrentADC() {
    // Select the correct ADC channel and read
    adc_select_input(adc_channel_);
    uint16_t adc_value = adc_read();
    
    // Clamp to valid range (shouldn't be necessary but good practice)
    if (adc_value >= POT_SCANNER_ADC_RESOLUTION) {
        adc_value = POT_SCANNER_ADC_RESOLUTION - 1;
    }
    
    return adc_value;
}

uint8_t PotScanner::mapValue(uint16_t adc_value) const {
    // Map from 0-4095 to 0-127 (MIDI range)
    // Using 32-bit math to avoid overflow
    uint32_t mapped = (adc_value * (POT_VALUE_MAX - POT_VALUE_MIN)) / 
                      (POT_SCANNER_ADC_RESOLUTION - 1);
    
    return (uint8_t)(mapped + POT_VALUE_MIN);
}