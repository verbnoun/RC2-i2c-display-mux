#pragma once

#include "io_types.h"
#include "drivers/analog_mux_driver.h"
#include "drivers/i2c_switch_driver.h"
#include "pico/sync.h"
#include <array>

// Main I/O Resource Controller - Traffic cop for all I/O expansion
class IOResourceController {
public:
    IOResourceController() = default;
    ~IOResourceController() = default;

    // Initialize all I/O resources with hardware configuration
    IOResult init(const IOHardwareConfig& config);
    
    // Generic resource request interface
    IOResult requestResource(IOResourceType type, uint8_t resource_id, 
                            uint8_t channel, IOPriority priority = IOPriority::MEDIUM);
    
    // High-level analog channel operations
    IOResult requestAnalogChannel(AnalogResourceID resource_id, uint8_t channel, 
                                 IOPriority priority = IOPriority::MEDIUM);
    
    // High-level I2C channel operations  
    IOResult requestI2CChannel(I2CResourceID resource_id, uint8_t channel, 
                              IOPriority priority = IOPriority::MEDIUM);
    
    // Release channel (lowers priority/allows other requests)
    IOResult releaseResource(IOResourceType type, uint8_t resource_id);
    
    // Direct access to underlying drivers (for advanced use)
    AnalogMuxDriver& getAnalogDriver() { return analog_driver_; }
    I2CSwitchDriver& getI2CDriver() { return i2c_driver_; }
    
    // Status queries
    bool isInitialized() const { return initialized_; }
    uint8_t getCurrentAnalogChannel(AnalogResourceID resource_id) const;
    uint8_t getCurrentI2CChannel(I2CResourceID resource_id) const;
    
    // Utility functions
    uint8_t getAnalogSignalPin(AnalogResourceID resource_id) const;
    i2c_inst_t* getI2CInstance(I2CResourceID resource_id) const;

private:
    AnalogMuxDriver analog_driver_;
    I2CSwitchDriver i2c_driver_;
    
    // Priority and request tracking
    std::array<IOResourceRequest, static_cast<size_t>(AnalogResourceID::MAX_ANALOG_RESOURCES)> analog_requests_;
    std::array<IOResourceRequest, static_cast<size_t>(I2CResourceID::MAX_I2C_RESOURCES)> i2c_requests_;
    
    bool initialized_;
    bool analog_driver_available_;
    bool i2c_driver_available_;
    mutex_t access_mutex_;  // Thread safety for multi-core operation
    
    // Internal priority management
    bool canOverridePriority(const IOResourceRequest& current, IOPriority new_priority) const;
    void updateRequestTracker(IOResourceRequest& request, uint8_t channel, IOPriority priority);
    uint32_t getCurrentTimeUs() const;
};