#pragma once

#include "../io_types.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include <array>

class I2CSwitchDriver {
public:
    I2CSwitchDriver() = default;
    ~I2CSwitchDriver() = default;

    // Initialize I2C switch with pin configuration
    IOResult init(const IOHardwareConfig& config);
    
    // Select channel on I2C switch
    IOResult selectChannel(I2CResourceID resource_id, uint8_t channel);
    
    // Disable all channels (useful for reset/cleanup)
    IOResult disableAllChannels(I2CResourceID resource_id);
    
    // Get currently selected channel
    uint8_t getCurrentChannel(I2CResourceID resource_id) const;
    
    // Check if switch is initialized
    bool isInitialized(I2CResourceID resource_id) const;
    
    // Get I2C instance for this switch
    i2c_inst_t* getI2CInstance(I2CResourceID resource_id) const;
    
    // Scan for devices on current channel
    bool scanCurrentChannel(I2CResourceID resource_id, uint8_t* found_addresses, uint8_t& count);
    
    // Utility function to validate channel range
    static bool isValidChannel(uint8_t channel);

private:
    struct I2CSwitchState {
        I2CSwitchPins pins;
        uint8_t current_channel;
        bool initialized;
        i2c_inst_t* i2c_instance;
    };
    
    std::array<I2CSwitchState, static_cast<size_t>(I2CResourceID::MAX_I2C_RESOURCES)> switch_states_;
    uint32_t i2c_timeout_us_;
    
    // Internal helper methods
    void configureI2CPins(const I2CSwitchPins& pins);
    IOResult writeChannelByte(const I2CSwitchState& state, uint8_t channel_byte);
    uint8_t getResourceIndex(I2CResourceID resource_id) const;
    i2c_inst_t* getI2CInstanceFromNumber(uint8_t instance_num) const;
    const char* getResourceName(I2CResourceID resource_id) const;
    bool detectTCA9548A(const I2CSwitchState& state);
};