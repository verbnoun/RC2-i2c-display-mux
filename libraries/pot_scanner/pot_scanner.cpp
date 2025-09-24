#include "pot_scanner.h"
#include <algorithm>
#include <cmath>

PotScanner::PotScanner() : current_channel_(0), scanning_active_(false), adc_channel_(0), 
                           current_external_channel_(0), total_scans_(0), last_rate_check_time_(0), 
                           scans_since_rate_check_(0) {
    // Initialize all channels as inactive with zero values
    for (uint8_t i = 0; i < POT_SCANNER_MAX_CHANNELS; i++) {
        channels_[i] = {0, 0.0f, false, 0, false};
    }
}

PotScanner::~PotScanner() {
    // Destructor - cleanup if needed
}

bool PotScanner::init(const pot_scanner_config_t& config) {
    LOG(TAG_POT, "Initializing hardware-focused pot scanner...");
    
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
    
    // Initialize all channel readings
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    for (uint8_t i = 0; i < POT_SCANNER_MAX_CHANNELS; i++) {
        pot_reading_t& reading = channels_[i];
        
        // Initialize hardware readings
        reading.raw_value = 0;
        reading.filtered_value = 0.0f;
        reading.is_valid = false;
        reading.timestamp = current_time;
        reading.is_active = true;  // All channels active by default
    }
    
    // Initialize scanner state
    current_channel_ = 0;
    scanning_active_ = false;
    total_scans_ = 0;
    last_rate_check_time_ = current_time;
    scans_since_rate_check_ = 0;
    
    LOG(TAG_POT, "Pot scanner initialized - %d channels, EMA alpha=%.2f, glitch threshold=%d", 
                 POT_SCANNER_MAX_CHANNELS, config_.ema_alpha, config_.glitch_threshold);
    
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
    if (current_time - channels_[current_channel_].timestamp < config_.scan_interval_ms) {
        return;
    }
    
    // Skip inactive channels
    if (!channels_[current_channel_].is_active) {
        // Move to next channel
        current_channel_ = (current_channel_ + 1) % POT_SCANNER_MAX_CHANNELS;
        return;
    }
    
    // Update the current channel
    updateChannel(current_channel_);
    
    // Move to next channel
    current_channel_ = (current_channel_ + 1) % POT_SCANNER_MAX_CHANNELS;
}

void PotScanner::updateChannel(uint8_t channel) {
    if (!isValidChannel(channel) || !channels_[channel].is_active) {
        return;
    }
    
    // External channel selection handled by caller (main.cpp)
    // Read current ADC value - channel should already be selected
    uint16_t raw_value = readCurrentADC();
    pot_reading_t& reading = channels_[channel];
    
    // Check for glitches (single outlier rejection)
    if (!isGlitch(channel, raw_value)) {
        // Update hardware reading
        reading.raw_value = raw_value;
        
        // Update EMA filter for electrical noise reduction
        updateEMA(channel, raw_value);
        
        // Validate range
        reading.is_valid = (raw_value <= POT_SCANNER_ADC_RESOLUTION);
        reading.timestamp = to_ms_since_boot(get_absolute_time());
        
        total_scans_++;
        scans_since_rate_check_++;
    } else {
        // Glitch detected - keep previous reading
        reading.is_valid = false;
        LOG(TAG_POT, "Ch%d: Glitch rejected (raw=%d vs ema=%.1f)", 
            channel, raw_value, reading.filtered_value);
    }
}

void PotScanner::updateEMA(uint8_t channel, uint16_t raw_value) {
    pot_reading_t& reading = channels_[channel];
    
    // Initialize EMA on first reading
    if (reading.filtered_value == 0.0f) {
        reading.filtered_value = (float)raw_value;
    } else {
        // Apply exponential moving average for electrical noise reduction
        reading.filtered_value = (config_.ema_alpha * raw_value) + 
                                ((1.0f - config_.ema_alpha) * reading.filtered_value);
    }
}

bool PotScanner::isGlitch(uint8_t channel, uint16_t raw_value) const {
    const pot_reading_t& reading = channels_[channel];
    
    // Skip glitch detection on first reading
    if (reading.filtered_value == 0.0f) {
        return false;
    }
    
    // Check if raw value differs too much from EMA (likely a glitch)
    float diff = fabsf((float)raw_value - reading.filtered_value);
    return diff > config_.glitch_threshold;
}

void PotScanner::setChannelEnabled(uint8_t channel, bool enabled) {
    if (!isValidChannel(channel)) {
        LOG(TAG_POT, "WARN: Invalid channel %d for enable/disable", channel);
        return;
    }
    
    channels_[channel].is_active = enabled;
    LOG(TAG_POT, "Channel %d %s", channel, enabled ? "enabled" : "disabled");
}

pot_reading_t PotScanner::getReading(uint8_t channel) const {
    if (!isValidChannel(channel)) {
        return {0, 0.0f, false, 0, false};
    }
    return channels_[channel];
}

uint16_t PotScanner::getRawValue(uint8_t channel) const {
    if (!isValidChannel(channel)) {
        return 0;
    }
    return channels_[channel].raw_value;
}

float PotScanner::getFilteredValue(uint8_t channel) const {
    if (!isValidChannel(channel)) {
        return 0.0f;
    }
    return channels_[channel].filtered_value;
}

void PotScanner::printStatus() const {
    LOG(TAG_POT, "=== Hardware Pot Scanner Status ===");
    LOG(TAG_POT, "Scanning: %s, Total scans: %lu, Rate: %.1f Hz", 
                 scanning_active_ ? "active" : "stopped", 
                 total_scans_, getCurrentScanRate());
    LOG(TAG_POT, "EMA alpha: %.2f, Glitch threshold: %d", 
                 config_.ema_alpha, config_.glitch_threshold);
    
    // Show active channels with current readings
    for (uint8_t i = 0; i < POT_SCANNER_MAX_CHANNELS; i++) {
        if (channels_[i].is_active) {
            const pot_reading_t& reading = channels_[i];
            LOG(TAG_POT, "Ch%2d: raw=%4d, filtered=%6.1f, %s", 
                         i, reading.raw_value, reading.filtered_value,
                         reading.is_valid ? "valid" : "INVALID");
        }
    }
}

void PotScanner::printChannelDiagnostics(uint8_t channel) const {
    if (!isValidChannel(channel)) {
        LOG(TAG_POT, "ERROR: Invalid channel %d for diagnostics", channel);
        return;
    }
    
    const pot_reading_t& reading = channels_[channel];
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    LOG(TAG_POT, "=== Channel %d Hardware Diagnostics ===", channel);
    LOG(TAG_POT, "Active: %s", reading.is_active ? "yes" : "no");
    LOG(TAG_POT, "Raw: %d, Filtered: %.1f, Valid: %s", 
                 reading.raw_value, reading.filtered_value, 
                 reading.is_valid ? "YES" : "NO");
    LOG(TAG_POT, "Last reading: %lu ms ago", 
                 current_time - reading.timestamp);
    LOG(TAG_POT, "Glitch threshold: %d ADC units", config_.glitch_threshold);
}

float PotScanner::getCurrentScanRate() const {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    uint32_t time_elapsed = current_time - last_rate_check_time_;
    
    if (time_elapsed == 0) {
        return 0.0f;
    }
    
    return (scans_since_rate_check_ * 1000.0f) / time_elapsed;
}

uint8_t PotScanner::getActivePotCount() const {
    uint8_t count = 0;
    for (uint8_t i = 0; i < POT_SCANNER_MAX_CHANNELS; i++) {
        if (channels_[i].is_active) {
            count++;
        }
    }
    return count;
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

