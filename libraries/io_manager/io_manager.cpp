#include "io_manager.h"
#include "pico/time.h"
#include "console_logger.h"

IOResult IOResourceController::init(const IOHardwareConfig& config) {
    LOG_INFO(TAG_MUX, "Initializing I/O Resource Controller...");
    
    // Initialize mutex for thread safety
    mutex_init(&access_mutex_);
    
    // Initialize analog multiplexer driver
    IOResult analog_result = analog_driver_.init(config);
    analog_driver_available_ = (analog_result == IOResult::SUCCESS);
    if (!analog_driver_available_) {
        LOG_ERROR(TAG_MUX, "Failed to initialize analog driver: %d", static_cast<int>(analog_result));
    }
    
    // Initialize I2C switch driver
    IOResult i2c_result = i2c_driver_.init(config);
    i2c_driver_available_ = (i2c_result == IOResult::SUCCESS);
    if (!i2c_driver_available_) {
        LOG_INFO(TAG_MUX, "I2C driver unavailable: %d (OLED mux will not work)", static_cast<int>(i2c_result));
    }
    
    // Require at least analog driver for basic operation
    if (!analog_driver_available_) {
        return analog_result;
    }
    
    // Initialize request trackers
    for (auto& request : analog_requests_) {
        request.type = IOResourceType::ANALOG_SIGNAL;
        request.channel = INVALID_CHANNEL;
        request.priority = IOPriority::LOW;
        request.active = false;
        request.timestamp_us = 0;
    }
    
    for (auto& request : i2c_requests_) {
        request.type = IOResourceType::I2C_BUS;
        request.channel = INVALID_CHANNEL;
        request.priority = IOPriority::LOW;
        request.active = false;
        request.timestamp_us = 0;
    }
    
    initialized_ = true;
    if (analog_driver_available_ && i2c_driver_available_) {
        LOG_INFO(TAG_MUX, "I/O Resource Controller ready - all drivers initialized");
    } else {
        LOG_INFO(TAG_MUX, "I/O Resource Controller ready - analog: %s, I2C: %s",
                 analog_driver_available_ ? "OK" : "FAIL",
                 i2c_driver_available_ ? "OK" : "FAIL");
    }
    return IOResult::SUCCESS;
}

IOResult IOResourceController::requestResource(IOResourceType type, uint8_t resource_id, 
                                              uint8_t channel, IOPriority priority) {
    switch (type) {
        case IOResourceType::ANALOG_SIGNAL:
            return requestAnalogChannel(static_cast<AnalogResourceID>(resource_id), channel, priority);
        case IOResourceType::I2C_BUS:
            return requestI2CChannel(static_cast<I2CResourceID>(resource_id), channel, priority);
        default:
            LOG_ERROR(TAG_MUX, "Unsupported resource type: %d", static_cast<int>(type));
            return IOResult::ERROR_INVALID_RESOURCE;
    }
}

IOResult IOResourceController::requestAnalogChannel(AnalogResourceID resource_id, uint8_t channel, IOPriority priority) {
    if (!initialized_) {
        return IOResult::ERROR_NOT_INITIALIZED;
    }
    
    if (!analog_driver_available_) {
        LOG_ERROR(TAG_MUX, "Cannot access analog mux - analog driver failed initialization");
        return IOResult::ERROR_HARDWARE_FAILURE;
    }
    
    uint8_t resource_index = static_cast<uint8_t>(resource_id);
    if (resource_index >= static_cast<uint8_t>(AnalogResourceID::MAX_ANALOG_RESOURCES)) {
        return IOResult::ERROR_INVALID_RESOURCE;
    }
    
    // Thread-safe access
    mutex_enter_blocking(&access_mutex_);
    
    auto& current_request = analog_requests_[resource_index];
    
    // Check if we can override current priority
    if (current_request.active && !canOverridePriority(current_request, priority)) {
        mutex_exit(&access_mutex_);
        return IOResult::ERROR_PRIORITY_CONFLICT;
    }
    
    // Perform the channel switch
    IOResult result = analog_driver_.selectChannel(resource_id, channel);
    
    if (result == IOResult::SUCCESS) {
        // Update request tracker
        updateRequestTracker(current_request, channel, priority);
        current_request.resource_id = resource_index;
    }
    
    mutex_exit(&access_mutex_);
    return result;
}

IOResult IOResourceController::requestI2CChannel(I2CResourceID resource_id, uint8_t channel, IOPriority priority) {
    if (!initialized_) {
        return IOResult::ERROR_NOT_INITIALIZED;
    }
    
    if (!i2c_driver_available_) {
        LOG_ERROR(TAG_MUX, "Cannot access I2C mux - I2C driver failed initialization");
        return IOResult::ERROR_HARDWARE_FAILURE;
    }
    
    uint8_t resource_index = static_cast<uint8_t>(resource_id);
    if (resource_index >= static_cast<uint8_t>(I2CResourceID::MAX_I2C_RESOURCES)) {
        return IOResult::ERROR_INVALID_RESOURCE;
    }
    
    // Thread-safe access
    mutex_enter_blocking(&access_mutex_);
    
    auto& current_request = i2c_requests_[resource_index];
    
    // Check if we can override current priority
    if (current_request.active && !canOverridePriority(current_request, priority)) {
        mutex_exit(&access_mutex_);
        return IOResult::ERROR_PRIORITY_CONFLICT;
    }
    
    // Perform the channel switch
    IOResult result = i2c_driver_.selectChannel(resource_id, channel);
    
    if (result == IOResult::SUCCESS) {
        // Update request tracker
        updateRequestTracker(current_request, channel, priority);
        current_request.resource_id = resource_index;
    }
    
    mutex_exit(&access_mutex_);
    return result;
}

IOResult IOResourceController::releaseResource(IOResourceType type, uint8_t resource_id) {
    if (!initialized_) {
        return IOResult::ERROR_NOT_INITIALIZED;
    }
    
    // Thread-safe access
    mutex_enter_blocking(&access_mutex_);
    
    if (type == IOResourceType::ANALOG_SIGNAL) {
        if (resource_id < static_cast<uint8_t>(AnalogResourceID::MAX_ANALOG_RESOURCES)) {
            analog_requests_[resource_id].active = false;
            analog_requests_[resource_id].priority = IOPriority::LOW;
        }
    } else if (type == IOResourceType::I2C_BUS) {
        if (resource_id < static_cast<uint8_t>(I2CResourceID::MAX_I2C_RESOURCES)) {
            i2c_requests_[resource_id].active = false;
            i2c_requests_[resource_id].priority = IOPriority::LOW;
        }
    }
    
    mutex_exit(&access_mutex_);
    return IOResult::SUCCESS;
}

uint8_t IOResourceController::getCurrentAnalogChannel(AnalogResourceID resource_id) const {
    return analog_driver_.getCurrentChannel(resource_id);
}

uint8_t IOResourceController::getCurrentI2CChannel(I2CResourceID resource_id) const {
    return i2c_driver_.getCurrentChannel(resource_id);
}

uint8_t IOResourceController::getAnalogSignalPin(AnalogResourceID resource_id) const {
    return analog_driver_.getSignalPin(resource_id);
}

i2c_inst_t* IOResourceController::getI2CInstance(I2CResourceID resource_id) const {
    return i2c_driver_.getI2CInstance(resource_id);
}

bool IOResourceController::canOverridePriority(const IOResourceRequest& current, IOPriority new_priority) const {
    // Higher priority values can override lower priority values
    // Equal priority is allowed (cooperative sharing)
    return static_cast<uint8_t>(new_priority) >= static_cast<uint8_t>(current.priority);
}

void IOResourceController::updateRequestTracker(IOResourceRequest& request, uint8_t channel, IOPriority priority) {
    request.channel = channel;
    request.priority = priority;
    request.timestamp_us = getCurrentTimeUs();
    request.active = true;
}

uint32_t IOResourceController::getCurrentTimeUs() const {
    return to_us_since_boot(get_absolute_time());
}