#pragma once

#include "../io_types.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include <array>

class AnalogMuxDriver {
public:
    AnalogMuxDriver() = default;
    ~AnalogMuxDriver() = default;

    // Initialize all analog multiplexers with pin configurations
    IOResult init(const IOHardwareConfig& config);
    
    // Select channel on specific analog multiplexer
    IOResult selectChannel(AnalogResourceID resource_id, uint8_t channel);
    
    // Get currently selected channel for a multiplexer
    uint8_t getCurrentChannel(AnalogResourceID resource_id) const;
    
    // Check if a multiplexer is initialized
    bool isInitialized(AnalogResourceID resource_id) const;
    
    // Get signal pin for reading ADC value
    uint8_t getSignalPin(AnalogResourceID resource_id) const;
    
    // Utility function to validate channel range
    static bool isValidChannel(uint8_t channel);

private:
    struct AnalogMuxState {
        AnalogMuxPins pins;
        uint8_t current_channel;
        bool initialized;
        std::array<bool, 4> select_pin_states; // Cache for S0-S3 states
    };
    
    std::array<AnalogMuxState, static_cast<size_t>(AnalogResourceID::MAX_ANALOG_RESOURCES)> mux_states_;
    uint32_t settling_time_us_;
    
    // Internal helper methods
    void configureSelectPins(const AnalogMuxPins& pins);
    void setChannelPins(const AnalogMuxPins& pins, uint8_t channel);
    uint8_t getResourceIndex(AnalogResourceID resource_id) const;
    const char* getResourceName(AnalogResourceID resource_id) const;
};