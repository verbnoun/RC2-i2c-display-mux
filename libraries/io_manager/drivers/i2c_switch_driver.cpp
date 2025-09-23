#include "i2c_switch_driver.h"
#include "pico/time.h"
#include "console_logger.h"

IOResult I2CSwitchDriver::init(const IOHardwareConfig& config) {
    i2c_timeout_us_ = config.i2c_timeout_us;
    
    LOG_INFO(TAG_MUX, "Initializing I2C switch driver...");
    LOG_INFO(TAG_MUX, "I2C timeout: %d us", i2c_timeout_us_);
    
    // Initialize OLED I2C switch
    auto& state = switch_states_[static_cast<size_t>(I2CResourceID::OLED_DISPLAYS)];
    state.pins = config.oled_pins;
    state.current_channel = INVALID_CHANNEL;
    state.initialized = false;
    
    // Get I2C instance
    state.i2c_instance = getI2CInstanceFromNumber(state.pins.i2c_instance);
    if (state.i2c_instance == nullptr) {
        LOG_ERROR(TAG_MUX, "  [%s] Invalid I2C instance %d", 
                      getResourceName(I2CResourceID::OLED_DISPLAYS),
                      state.pins.i2c_instance);
        return IOResult::ERROR_INVALID_RESOURCE;
    }
    
    LOG_INFO(TAG_MUX, "  [%s] TCA9548A on I2C%d (GP%d:SDA, GP%d:SCL) @ 0x%02X",
                 getResourceName(I2CResourceID::OLED_DISPLAYS),
                 state.pins.i2c_instance,
                 state.pins.sda_pin,
                 state.pins.scl_pin,
                 state.pins.address);
    
    // Configure I2C pins and initialize
    configureI2CPins(state.pins);
    
    // Initialize I2C with 400kHz (fast mode)
    i2c_init(state.i2c_instance, 400000);
    LOG_INFO(TAG_MUX, "    I2C bus initialized at 400kHz");
    
    // Detect TCA9548A
    if (detectTCA9548A(state)) {
        LOG_INFO(TAG_MUX, "    ✓ TCA9548A detected and responding");
        
        // Disable all channels initially
        IOResult result = disableAllChannels(I2CResourceID::OLED_DISPLAYS);
        if (result == IOResult::SUCCESS) {
            state.initialized = true;
            LOG_INFO(TAG_MUX, "    ✓ All channels disabled, switch ready");
            
            // Quick scan to see what's connected
            uint8_t addresses[8];
            uint8_t count = 0;
            for (uint8_t ch = 0; ch < MAX_CHANNELS_PER_TCA9548A; ch++) {
                if (selectChannel(I2CResourceID::OLED_DISPLAYS, ch) == IOResult::SUCCESS) {
                    if (scanCurrentChannel(I2CResourceID::OLED_DISPLAYS, addresses, count)) {
                        LOG_INFO(TAG_MUX, "    Channel %d: %d device(s) found", ch, count);
                    }
                }
            }
            disableAllChannels(I2CResourceID::OLED_DISPLAYS);
        } else {
            LOG_ERROR(TAG_MUX, "    ✗ Failed to disable channels");
            return result;
        }
    } else {
        LOG_ERROR(TAG_MUX, "    ✗ TCA9548A not detected at 0x%02X", state.pins.address);
        return IOResult::ERROR_HARDWARE_FAILURE;
    }
    
    LOG_INFO(TAG_MUX, "I2C switch driver ready");
    return IOResult::SUCCESS;
}

IOResult I2CSwitchDriver::selectChannel(I2CResourceID resource_id, uint8_t channel) {
    if (!isValidChannel(channel)) {
        LOG_ERROR(TAG_MUX, "[%s] Invalid channel %d (max: %d)",
                      getResourceName(resource_id), channel, MAX_CHANNELS_PER_TCA9548A-1);
        return IOResult::ERROR_INVALID_CHANNEL;
    }
    
    uint8_t resource_index = getResourceIndex(resource_id);
    if (resource_index >= static_cast<uint8_t>(I2CResourceID::MAX_I2C_RESOURCES)) {
        return IOResult::ERROR_INVALID_RESOURCE;
    }
    
    auto& state = switch_states_[resource_index];
    if (!state.initialized) {
        return IOResult::ERROR_NOT_INITIALIZED;
    }
    
    // Skip if already on the correct channel
    if (state.current_channel == channel) {
        return IOResult::SUCCESS;
    }
    
    // TCA9548A channel selection: write single byte with bit set for desired channel
    uint8_t channel_byte = 1 << channel;
    
    IOResult result = writeChannelByte(state, channel_byte);
    if (result == IOResult::SUCCESS) {
        state.current_channel = channel;
    }
    
    return result;
}

IOResult I2CSwitchDriver::disableAllChannels(I2CResourceID resource_id) {
    uint8_t resource_index = getResourceIndex(resource_id);
    if (resource_index >= static_cast<uint8_t>(I2CResourceID::MAX_I2C_RESOURCES)) {
        return IOResult::ERROR_INVALID_RESOURCE;
    }
    
    auto& state = switch_states_[resource_index];
    
    // Write 0 to disable all channels
    IOResult result = writeChannelByte(state, 0x00);
    if (result == IOResult::SUCCESS) {
        state.current_channel = INVALID_CHANNEL;
    }
    
    return result;
}

uint8_t I2CSwitchDriver::getCurrentChannel(I2CResourceID resource_id) const {
    uint8_t resource_index = getResourceIndex(resource_id);
    if (resource_index >= static_cast<uint8_t>(I2CResourceID::MAX_I2C_RESOURCES)) {
        return INVALID_CHANNEL;
    }
    
    return switch_states_[resource_index].current_channel;
}

bool I2CSwitchDriver::isInitialized(I2CResourceID resource_id) const {
    uint8_t resource_index = getResourceIndex(resource_id);
    if (resource_index >= static_cast<uint8_t>(I2CResourceID::MAX_I2C_RESOURCES)) {
        return false;
    }
    
    return switch_states_[resource_index].initialized;
}

i2c_inst_t* I2CSwitchDriver::getI2CInstance(I2CResourceID resource_id) const {
    uint8_t resource_index = getResourceIndex(resource_id);
    if (resource_index >= static_cast<uint8_t>(I2CResourceID::MAX_I2C_RESOURCES)) {
        return nullptr;
    }
    
    return switch_states_[resource_index].i2c_instance;
}

bool I2CSwitchDriver::scanCurrentChannel(I2CResourceID resource_id, uint8_t* found_addresses, uint8_t& count) {
    count = 0;
    uint8_t resource_index = getResourceIndex(resource_id);
    if (resource_index >= static_cast<uint8_t>(I2CResourceID::MAX_I2C_RESOURCES)) {
        return false;
    }
    
    auto& state = switch_states_[resource_index];
    if (!state.initialized) {
        return false;
    }
    
    // Scan common I2C addresses
    const uint8_t scan_addresses[] = {0x3C, 0x3D}; // Common OLED addresses
    for (uint8_t addr : scan_addresses) {
        uint8_t data;
        int result = i2c_read_timeout_us(state.i2c_instance, addr, &data, 1, false, 10000);
        if (result >= 0) {
            found_addresses[count++] = addr;
        }
    }
    
    return count > 0;
}

bool I2CSwitchDriver::isValidChannel(uint8_t channel) {
    return channel < MAX_CHANNELS_PER_TCA9548A;
}

void I2CSwitchDriver::configureI2CPins(const I2CSwitchPins& pins) {
    // Configure I2C pins
    gpio_set_function(pins.sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(pins.scl_pin, GPIO_FUNC_I2C);
    
    // Enable pull-ups (important for I2C)
    gpio_pull_up(pins.sda_pin);
    gpio_pull_up(pins.scl_pin);
}

IOResult I2CSwitchDriver::writeChannelByte(const I2CSwitchState& state, uint8_t channel_byte) {
    // Perform I2C write with timeout
    int result = i2c_write_timeout_us(
        state.i2c_instance,
        state.pins.address,
        &channel_byte,
        1,
        false,  // nostop = false (send stop condition)
        i2c_timeout_us_
    );
    
    switch (result) {
        case 1:  // Success: 1 byte written
            return IOResult::SUCCESS;
        case PICO_ERROR_GENERIC:
            return IOResult::ERROR_I2C_NACK;
        case PICO_ERROR_TIMEOUT:
            return IOResult::ERROR_I2C_TIMEOUT;
        default:
            return IOResult::ERROR_I2C_NACK;
    }
}

uint8_t I2CSwitchDriver::getResourceIndex(I2CResourceID resource_id) const {
    return static_cast<uint8_t>(resource_id);
}

i2c_inst_t* I2CSwitchDriver::getI2CInstanceFromNumber(uint8_t instance_num) const {
    switch (instance_num) {
        case 0:
            return i2c0;
        case 1:
            return i2c1;
        default:
            return nullptr;
    }
}

const char* I2CSwitchDriver::getResourceName(I2CResourceID resource_id) const {
    switch (resource_id) {
        case I2CResourceID::OLED_DISPLAYS: return "OLED-MUX";
        default: return "UNKNOWN";
    }
}

bool I2CSwitchDriver::detectTCA9548A(const I2CSwitchState& state) {
    // Try to read from the TCA9548A
    uint8_t data;
    int result = i2c_read_timeout_us(
        state.i2c_instance,
        state.pins.address,
        &data,
        1,
        false,
        10000  // 10ms timeout for detection
    );
    
    return result >= 0;
}