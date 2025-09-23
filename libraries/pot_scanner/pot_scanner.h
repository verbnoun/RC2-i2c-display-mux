#ifndef POT_SCANNER_H
#define POT_SCANNER_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "console_logger.h"

// Default configuration constants
#define POT_SCANNER_MAX_CHANNELS 16
#define POT_SCANNER_ADC_RESOLUTION 4096  // 12-bit ADC

// Music production optimized algorithm constants
#define POT_SCANNER_DEFAULT_MOVEMENT_THRESHOLD 2    // ADC units for movement detection (music production sensitive)
#define POT_SCANNER_DEFAULT_QUIET_THRESHOLD 8      // ADC units for quiescence detection (faster settling)
#define POT_SCANNER_DEFAULT_QUIET_TIME_MS 200      // 200ms before considering pot quiet (faster stop)
#define POT_SCANNER_DEFAULT_EMA_ALPHA 0.15f        // Exponential moving average smoothing (faster settling)
#define POT_SCANNER_DEFAULT_STABILITY_REQUIRED 1   // Stability count before reporting
#define POT_SCANNER_DEFAULT_MIN_REPORT_INTERVAL_MS 50  // Minimum time between reports
#define POT_SCANNER_DEFAULT_SCAN_INTERVAL_MS 10    // Default scan interval

// Value mapping ranges
#define POT_VALUE_MIN 0
#define POT_VALUE_MAX 127  // MIDI-compatible range

// Simplified pot scanner configuration (ADC only)
typedef struct {
    uint adc_pin;                      // ADC signal input pin (GP26-29)
    uint16_t movement_threshold;       // Movement detection threshold (ADC units)
    uint16_t quiet_threshold;          // Quiescence detection threshold (ADC units)
    uint32_t quiet_time_ms;           // Time before considering pot quiet
    float ema_alpha;                   // EMA smoothing factor
    uint8_t stability_required;        // Stability count before reporting
    uint32_t min_report_interval_ms;   // Minimum time between reports
    uint32_t scan_interval_ms;         // Scan interval for automatic scanning
} pot_scanner_config_t;

// Individual pot state (matches implementation)
typedef struct {
    // Current values
    uint16_t raw_value;           // Current raw ADC reading
    uint16_t current_value;       // Current processed value
    uint16_t previous_value;      // Previous value for delta calculation
    uint16_t last_reported_value; // Last value that was reported
    uint8_t mapped_value;         // Mapped to MIDI range (0-127)
    float ema_value;              // EMA filtered value
    
    // Change detection and timing
    bool has_changed;             // Flag indicating value changed
    uint32_t last_change_time;    // Time of last change
    uint32_t last_report_time;    // Time of last report
    uint32_t last_scan_time;      // Time of last scan
    uint32_t last_movement_time;  // Time of last movement
    bool is_quiet;                // Quiet state flag
    
    // Movement tracking
    int8_t last_direction;        // Last movement direction
    uint8_t direction_consistency; // Direction consistency count
    uint8_t stability_count;      // Stability count
    uint16_t min_value;           // Minimum value seen
    uint16_t max_value;           // Maximum value seen
    bool is_active;               // Channel active flag
} pot_state_t;

class PotScanner {
public:
    PotScanner();
    ~PotScanner();
    
    // Configuration and initialization
    bool init(const pot_scanner_config_t& config);
    void startScanning();
    void stopScanning();
    
    // Main processing (call regularly from main loop) 
    void update();
    
    // Update specific pot channel (for external orchestration)
    void updatePot(uint8_t channel);
    
    // External channel selection and ADC reading (for main.cpp orchestration)
    void selectExternalChannel(uint8_t channel);
    uint16_t readCurrentADC();
    
    // Value access
    bool hasValueChanged(uint8_t channel);
    uint8_t getValue(uint8_t channel) const;          // Get mapped value (0-127)
    uint16_t getRawValue(uint8_t channel) const;      // Get raw ADC value
    
    // Status and diagnostics
    void printStatus() const;
    void printChannelDiagnostics(uint8_t channel) const;
    void calibrate();
    float getCurrentScanRate() const;
    void clearChangedFlags();
    bool anyPotChanged() const;
    uint8_t getActivePotCount() const;
    void setChannelEnabled(uint8_t channel, bool enabled);
    
private:
    pot_scanner_config_t config_;
    pot_state_t channels_[POT_SCANNER_MAX_CHANNELS];
    uint8_t current_channel_;
    bool scanning_active_;
    uint8_t adc_channel_;
    uint8_t current_external_channel_;
    uint32_t total_scans_;
    uint32_t last_rate_check_time_;
    uint32_t scans_since_rate_check_;
    
    // Internal processing methods
    void updateEMA(uint8_t channel, uint16_t raw_value);
    void updateQuiescence(uint8_t channel);
    uint8_t mapValue(uint16_t adc_value) const;
    bool isValidChannel(uint8_t channel) const { return channel < POT_SCANNER_MAX_CHANNELS; }
};

#endif // POT_SCANNER_H