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

// Hardware-focused constants (electrical noise filtering only)
#define POT_SCANNER_DEFAULT_EMA_ALPHA 0.15f        // Exponential moving average for electrical noise
#define POT_SCANNER_DEFAULT_GLITCH_THRESHOLD 4095  // Disable glitch rejection - accept all values
#define POT_SCANNER_DEFAULT_SCAN_INTERVAL_MS 10    // Default scan interval

// Value mapping ranges
#define POT_VALUE_MIN 0
#define POT_VALUE_MAX 127  // MIDI-compatible range

// Hardware-focused pot scanner configuration
typedef struct {
    uint adc_pin;                      // ADC signal input pin (GP26-29)
    float ema_alpha;                   // EMA smoothing factor for electrical noise
    uint16_t glitch_threshold;         // Single-sample outlier rejection threshold
    uint32_t scan_interval_ms;         // Scan interval for automatic scanning
} pot_scanner_config_t;

// Hardware reading data (clean, filtered ADC values)
typedef struct {
    uint16_t raw_value;           // Direct ADC reading
    float filtered_value;         // EMA filtered value (electrical noise removed)
    bool is_valid;                // Passed range and glitch checks
    uint32_t timestamp;           // Time of reading
    bool is_active;               // Channel enabled flag
} pot_reading_t;

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
    
    // External channel selection and reading (for main.cpp orchestration)
    void selectExternalChannel(uint8_t channel);
    uint16_t readCurrentADC();
    void updateChannel(uint8_t channel);              // Update specific channel reading
    
    // Value access (hardware data only)
    pot_reading_t getReading(uint8_t channel) const;  // Get complete reading data
    uint16_t getRawValue(uint8_t channel) const;      // Get raw ADC value
    float getFilteredValue(uint8_t channel) const;    // Get EMA filtered value
    
    // Status and diagnostics
    void printStatus() const;
    void printChannelDiagnostics(uint8_t channel) const;
    float getCurrentScanRate() const;
    uint8_t getActivePotCount() const;
    void setChannelEnabled(uint8_t channel, bool enabled);
    
private:
    pot_scanner_config_t config_;
    pot_reading_t channels_[POT_SCANNER_MAX_CHANNELS];
    uint8_t current_channel_;
    bool scanning_active_;
    uint8_t adc_channel_;
    uint8_t current_external_channel_;
    uint32_t total_scans_;
    uint32_t last_rate_check_time_;
    uint32_t scans_since_rate_check_;
    
    // Internal processing methods
    void updateEMA(uint8_t channel, uint16_t raw_value);
    bool isGlitch(uint8_t channel, uint16_t raw_value) const;
    bool isValidChannel(uint8_t channel) const { return channel < POT_SCANNER_MAX_CHANNELS; }
};

#endif // POT_SCANNER_H