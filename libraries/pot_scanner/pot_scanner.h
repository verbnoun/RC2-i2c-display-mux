#ifndef POT_SCANNER_H
#define POT_SCANNER_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "console_logger.h"

// Default configuration constants
#define POT_SCANNER_MAX_CHANNELS 16
#define POT_SCANNER_ADC_RESOLUTION 4096  // 12-bit ADC
#define POT_SCANNER_DEFAULT_SETTLING_TIME_US 1000   // 1ms settling time (reduced from 10ms)
#define POT_SCANNER_DEFAULT_SCAN_INTERVAL_MS 5      // 5ms scan interval per channel (reduced from 50ms)

// Music production optimized algorithm constants
#define POT_SCANNER_DEFAULT_MOVEMENT_THRESHOLD 2    // ADC units for movement detection (music production sensitive)
#define POT_SCANNER_DEFAULT_QUIET_THRESHOLD 8      // ADC units for quiescence detection (faster settling)
#define POT_SCANNER_DEFAULT_QUIET_TIME_MS 200      // 200ms before considering pot quiet (faster stop)
#define POT_SCANNER_DEFAULT_EMA_ALPHA 0.15f        // Exponential moving average smoothing (faster settling)
#define POT_SCANNER_DEFAULT_STABILITY_REQUIRED 1   // Stability count before reporting
#define POT_SCANNER_DEFAULT_MIN_REPORT_INTERVAL_MS 50  // Minimum time between reports

// Value mapping ranges
#define POT_VALUE_MIN 0
#define POT_VALUE_MAX 127  // MIDI-compatible range

// Pot scanner configuration structure
typedef struct {
    uint sig_pin;              // ADC signal input pin (GP26-29)
    uint s0_pin;               // Address bit 0
    uint s1_pin;               // Address bit 1  
    uint s2_pin;               // Address bit 2
    uint s3_pin;               // Address bit 3
    uint settling_time_us;     // Channel switching settling time
    uint scan_interval_ms;     // Scan rate per channel
    uint16_t movement_threshold;   // Movement detection threshold (ADC units)
    uint16_t quiet_threshold;      // Quiescence detection threshold (ADC units)
    uint32_t quiet_time_ms;        // Time before considering pot quiet
    float ema_alpha;               // EMA smoothing factor
    uint8_t stability_required;    // Stability count before reporting
    uint32_t min_report_interval_ms; // Minimum time between reports
} pot_scanner_config_t;

// Individual pot state (based on proven algorithm)
typedef struct {
    // Current values
    uint16_t raw_value;         // Current raw ADC reading
    uint16_t current_value;     // Current processed value
    uint16_t previous_value;    // Previous processed value
    uint16_t last_reported_value; // Last value reported as changed
    uint8_t mapped_value;       // Mapped value (0-127)
    
    // EMA filtering
    float ema_value;            // Exponential moving average
    
    // Change detection
    bool has_changed;           // Flag indicating significant change since last check
    uint32_t last_change_time;  // Time of last reported change
    uint32_t last_report_time;  // Time of last report
    uint32_t last_scan_time;    // Last scan timestamp
    
    // Movement and quiescence detection
    uint32_t last_movement_time; // Time of last significant movement
    bool is_quiet;              // Channel is quiet (no significant movement)
    
    // Direction and stability tracking
    int8_t last_direction;      // Last movement direction (-1, 0, +1)
    uint8_t direction_consistency; // Count of consistent direction movements
    uint8_t stability_count;    // Count toward stability requirement
    
    // Range tracking
    uint16_t min_value;         // Minimum value seen
    uint16_t max_value;         // Maximum value seen
    
    bool is_active;             // Channel is enabled for scanning
} pot_state_t;

class PotScanner {
public:
    /**
     * Initialize the pot scanner with hardware configuration
     * @param config Hardware pin configuration and settings
     * @return true if initialization successful
     */
    bool init(const pot_scanner_config_t& config);
    
    /**
     * Start continuous scanning in background
     * Call update() regularly to process channels
     */
    void startScanning();
    
    /**
     * Stop scanning
     */
    void stopScanning();
    
    /**
     * Update pot scanner - call regularly from main loop
     * Processes one channel per call for non-blocking operation
     */
    void update();
    
    /**
     * Enable/disable a specific channel
     * @param channel Channel number (0-15)
     * @param enabled Enable state
     */
    void setChannelEnabled(uint8_t channel, bool enabled);
    
    /**
     * Get current mapped value for a channel (0-127)
     * @param channel Channel number (0-15)
     * @return Mapped pot value or 0 if invalid channel
     */
    uint8_t getValue(uint8_t channel) const;
    
    /**
     * Get raw ADC value for a channel
     * @param channel Channel number (0-15)
     * @return Raw ADC value (0-4095) or 0 if invalid channel
     */
    uint16_t getRawValue(uint8_t channel) const;
    
    /**
     * Check if a channel value has changed significantly
     * @param channel Channel number (0-15)
     * @return true if value changed since last check
     */
    bool hasValueChanged(uint8_t channel);
    
    /**
     * Print current status of all channels to console
     */
    void printStatus() const;
    
    /**
     * Print detailed diagnostics for a specific channel
     * @param channel Channel number (0-15)
     */
    void printChannelDiagnostics(uint8_t channel) const;
    
    /**
     * Get total number of scans performed
     */
    uint32_t getTotalScans() const { return total_scans_; }
    
    /**
     * Get current scan rate in Hz
     */
    float getCurrentScanRate() const;
    
    /**
     * Clear all changed flags (call after processing changes)
     */
    void clearChangedFlags();
    
    /**
     * Check if any pot has changed
     */
    bool anyPotChanged() const;
    
    /**
     * Get count of recently active pots
     */
    uint8_t getActivePotCount() const;
    
    /**
     * Calibrate all pots (reset baselines)
     */
    void calibrate();

private:
    pot_scanner_config_t config_;
    pot_state_t channels_[POT_SCANNER_MAX_CHANNELS];
    uint8_t current_channel_;
    bool scanning_active_;
    uint32_t total_scans_;
    uint32_t last_rate_check_time_;
    uint32_t scans_since_rate_check_;
    
    /**
     * Select a specific multiplexer channel
     * @param channel Channel number (0-15)
     */
    void selectChannel(uint8_t channel);
    
    /**
     * Read ADC value from currently selected channel
     * @return Raw ADC value (0-4095)
     */
    uint16_t readADC();
    
    /**
     * Update individual pot with new reading (core algorithm)
     * @param channel Channel number
     */
    void updatePot(uint8_t channel);
    
    /**
     * Update EMA filter for a channel
     * @param channel Channel number
     * @param raw_value New raw ADC reading
     */
    void updateEMA(uint8_t channel, uint16_t raw_value);
    
    /**
     * Update quiescence detection for a channel
     * @param channel Channel number
     */
    void updateQuiescence(uint8_t channel);
    
    /**
     * Check if a change is significant enough to report
     * @param channel Channel number
     * @param new_value New processed value
     * @return true if change should be reported
     */
    bool isSignificantChange(uint8_t channel, uint16_t new_value);
    
    /**
     * Map ADC value to output range (0-127)
     * @param adc_value Raw or filtered ADC value
     * @return Mapped value
     */
    uint8_t mapValue(uint16_t adc_value) const;
    
    /**
     * Validate channel number
     * @param channel Channel number to validate
     * @return true if valid (0-15)
     */
    bool isValidChannel(uint8_t channel) const {
        return channel < POT_SCANNER_MAX_CHANNELS;
    }
};

// Convenience macros for pot scanner logging
#define LOG_POT_DEBUG(...)   LOG_DEBUG(TAG_POT, __VA_ARGS__)
#define LOG_POT_INFO(...)    LOG_INFO(TAG_POT, __VA_ARGS__)
#define LOG_POT_WARN(...)    LOG_WARN(TAG_POT, __VA_ARGS__)
#define LOG_POT_ERROR(...)   LOG_ERROR(TAG_POT, __VA_ARGS__)

#define LOG_MUX_DEBUG(...)   LOG_DEBUG(TAG_MUX, __VA_ARGS__)
#define LOG_MUX_INFO(...)    LOG_INFO(TAG_MUX, __VA_ARGS__)
#define LOG_MUX_WARN(...)    LOG_WARN(TAG_MUX, __VA_ARGS__)
#define LOG_MUX_ERROR(...)   LOG_ERROR(TAG_MUX, __VA_ARGS__)

#endif // POT_SCANNER_H