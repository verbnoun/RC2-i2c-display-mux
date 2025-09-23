#include "analog_mux_driver.h"
#include "pico/time.h"
#include "console_logger.h"

IOResult AnalogMuxDriver::init(const IOHardwareConfig& config) {
    settling_time_us_ = config.analog_settling_time_us;
    
    LOG_INFO(TAG_MUX, "Initializing analog multiplexer driver...");
    LOG_INFO(TAG_MUX, "Settling time: %d us", settling_time_us_);
    
    // Initialize each analog multiplexer
    const AnalogMuxPins* pin_configs[] = {
        &config.kb_l1a_pins,   // AnalogResourceID::KB_L1A
        &config.kb_l1b_pins,   // AnalogResourceID::KB_L1B
        &config.kb_l2_pins,    // AnalogResourceID::KB_L2
        &config.pots_pins,     // AnalogResourceID::POTS
        &config.spare_pins     // AnalogResourceID::SPARE
    };
    
    for (size_t i = 0; i < static_cast<size_t>(AnalogResourceID::MAX_ANALOG_RESOURCES); ++i) {
        auto& state = mux_states_[i];
        state.pins = *pin_configs[i];
        state.current_channel = INVALID_CHANNEL;
        state.initialized = false;
        
        // Skip if pins are not configured (0xFF = unused)
        if (state.pins.s0_pin == 0xFF) {
            LOG_INFO(TAG_MUX, "  [%s] Not configured (skipped)", 
                         getResourceName(static_cast<AnalogResourceID>(i)));
            continue;
        }
        
        // Log the configuration
        LOG_INFO(TAG_MUX, "  [%s] CD74HC4067 on pins S0-S3: GP%d,GP%d,GP%d,GP%d", 
                     getResourceName(static_cast<AnalogResourceID>(i)),
                     state.pins.s0_pin, state.pins.s1_pin, 
                     state.pins.s2_pin, state.pins.s3_pin);
        
        if (state.pins.sig_pin != 0xFF) {
            LOG_INFO(TAG_MUX, "    Signal pin: GP%d (ADC%d)", 
                         state.pins.sig_pin, 
                         (state.pins.sig_pin >= 26 && state.pins.sig_pin <= 29) ? 
                         state.pins.sig_pin - 26 : -1);
        }
        
        // Configure GPIO pins
        configureSelectPins(state.pins);
        
        // Initialize to channel 0 and test
        setChannelPins(state.pins, 0);
        state.current_channel = 0;
        state.initialized = true;
        
        LOG_INFO(TAG_MUX, "    ✓ Initialized and set to channel 0");
    }
    
    LOG_INFO(TAG_MUX, "Analog multiplexer driver ready");
    return IOResult::SUCCESS;
}

IOResult AnalogMuxDriver::selectChannel(AnalogResourceID resource_id, uint8_t channel) {
    if (!isValidChannel(channel)) {
        LOG_ERROR(TAG_MUX, "[%s] Invalid channel %d (max: %d)", 
                      getResourceName(resource_id), channel, MAX_CHANNELS_PER_CD74HC4067-1);
        return IOResult::ERROR_INVALID_CHANNEL;
    }
    
    uint8_t resource_index = getResourceIndex(resource_id);
    if (resource_index >= static_cast<uint8_t>(AnalogResourceID::MAX_ANALOG_RESOURCES)) {
        return IOResult::ERROR_INVALID_RESOURCE;
    }
    
    auto& state = mux_states_[resource_index];
    if (!state.initialized) {
        return IOResult::ERROR_NOT_INITIALIZED;
    }
    
    // Skip if already on the correct channel
    if (state.current_channel == channel) {
        return IOResult::SUCCESS;
    }
    
    // Set new channel
    setChannelPins(state.pins, channel);
    state.current_channel = channel;
    
    // Allow settling time
    busy_wait_us(settling_time_us_);
    
    return IOResult::SUCCESS;
}

uint8_t AnalogMuxDriver::getCurrentChannel(AnalogResourceID resource_id) const {
    uint8_t resource_index = getResourceIndex(resource_id);
    if (resource_index >= static_cast<uint8_t>(AnalogResourceID::MAX_ANALOG_RESOURCES)) {
        return INVALID_CHANNEL;
    }
    
    return mux_states_[resource_index].current_channel;
}

bool AnalogMuxDriver::isInitialized(AnalogResourceID resource_id) const {
    uint8_t resource_index = getResourceIndex(resource_id);
    if (resource_index >= static_cast<uint8_t>(AnalogResourceID::MAX_ANALOG_RESOURCES)) {
        return false;
    }
    
    return mux_states_[resource_index].initialized;
}

uint8_t AnalogMuxDriver::getSignalPin(AnalogResourceID resource_id) const {
    uint8_t resource_index = getResourceIndex(resource_id);
    if (resource_index >= static_cast<uint8_t>(AnalogResourceID::MAX_ANALOG_RESOURCES)) {
        return 0xFF; // Invalid pin
    }
    
    return mux_states_[resource_index].pins.sig_pin;
}

bool AnalogMuxDriver::isValidChannel(uint8_t channel) {
    return channel < MAX_CHANNELS_PER_CD74HC4067;
}

void AnalogMuxDriver::configureSelectPins(const AnalogMuxPins& pins) {
    // Configure select pins as outputs
    gpio_init(pins.s0_pin);
    gpio_set_dir(pins.s0_pin, GPIO_OUT);
    
    gpio_init(pins.s1_pin);
    gpio_set_dir(pins.s1_pin, GPIO_OUT);
    
    gpio_init(pins.s2_pin);
    gpio_set_dir(pins.s2_pin, GPIO_OUT);
    
    gpio_init(pins.s3_pin);
    gpio_set_dir(pins.s3_pin, GPIO_OUT);
    
    // Signal pin is typically configured as ADC input by the application
    // We don't configure it here to avoid conflicts
}

void AnalogMuxDriver::setChannelPins(const AnalogMuxPins& pins, uint8_t channel) {
    // Set select pins based on channel (binary representation)
    gpio_put(pins.s0_pin, (channel & 0x01) != 0);  // Bit 0
    gpio_put(pins.s1_pin, (channel & 0x02) != 0);  // Bit 1
    gpio_put(pins.s2_pin, (channel & 0x04) != 0);  // Bit 2
    gpio_put(pins.s3_pin, (channel & 0x08) != 0);  // Bit 3
}

uint8_t AnalogMuxDriver::getResourceIndex(AnalogResourceID resource_id) const {
    return static_cast<uint8_t>(resource_id);
}

const char* AnalogMuxDriver::getResourceName(AnalogResourceID resource_id) const {
    switch (resource_id) {
        case AnalogResourceID::KB_L1A: return "KB-L1A";
        case AnalogResourceID::KB_L1B: return "KB-L1B";
        case AnalogResourceID::KB_L2:  return "KB-L2";
        case AnalogResourceID::POTS:   return "POTS";
        case AnalogResourceID::SPARE:  return "SPARE";
        default: return "UNKNOWN";
    }
}