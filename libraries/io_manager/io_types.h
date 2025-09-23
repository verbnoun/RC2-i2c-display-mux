#pragma once

#include <cstdint>

// Priority levels for I/O resource operations
enum class IOPriority : uint8_t {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    CRITICAL = 3
};

// Types of I/O resources managed
enum class IOResourceType : uint8_t {
    ANALOG_SIGNAL,      // Analog signal routing (CD74HC4067)
    I2C_BUS,           // I2C bus switching (TCA9548A)
    SPI_DEVICE,        // Future: SPI chip select management
    GPIO_BANK,         // Future: GPIO expansion (MCP23017, etc.)
    DIGITAL_SIGNAL     // Future: Digital signal routing
};

// Analog signal resource identifiers (CD74HC4067 instances)
enum class AnalogResourceID : uint8_t {
    KB_L1A = 0,        // Keyboard Layer 1A signals
    KB_L1B = 1,        // Keyboard Layer 1B signals  
    KB_L2 = 2,         // Keyboard Layer 2 signals
    POTS = 3,          // Potentiometer signals
    SPARE = 4,         // Fifth CD74HC4067 (spare/future use)
    MAX_ANALOG_RESOURCES = 5
};

// I2C bus resource identifiers (TCA9548A instances)
enum class I2CResourceID : uint8_t {
    OLED_DISPLAYS = 0,
    MAX_I2C_RESOURCES = 1
};

// Pin configuration for a single CD74HC4067 analog multiplexer
struct AnalogMuxPins {
    uint8_t s0_pin;     // Select bit 0
    uint8_t s1_pin;     // Select bit 1  
    uint8_t s2_pin;     // Select bit 2
    uint8_t s3_pin;     // Select bit 3
    uint8_t sig_pin;    // Signal/output pin (ADC input)
};

// Pin configuration for TCA9548A I2C multiplexer
struct I2CSwitchPins {
    uint8_t sda_pin;    // I2C SDA pin
    uint8_t scl_pin;    // I2C SCL pin
    uint8_t address;    // I2C address (typically 0x70)
    uint8_t i2c_instance; // I2C instance (0 or 1)
};

// Complete hardware configuration for all I/O resources
struct IOHardwareConfig {
    // Analog multiplexer configurations
    AnalogMuxPins kb_l1a_pins;
    AnalogMuxPins kb_l1b_pins;
    AnalogMuxPins kb_l2_pins;
    AnalogMuxPins pots_pins;
    AnalogMuxPins spare_pins;
    
    // I2C switch configuration
    I2CSwitchPins oled_pins;
    
    // Timing configurations
    uint32_t analog_settling_time_us;   // Settling time after channel switch
    uint32_t i2c_timeout_us;           // I2C operation timeout
};

// Operation result codes
enum class IOResult : uint8_t {
    SUCCESS = 0,
    ERROR_INVALID_RESOURCE,
    ERROR_INVALID_CHANNEL,
    ERROR_I2C_TIMEOUT,
    ERROR_I2C_NACK,
    ERROR_PRIORITY_CONFLICT,
    ERROR_NOT_INITIALIZED,
    ERROR_HARDWARE_FAILURE
};

// Generic I/O resource request
struct IOResourceRequest {
    IOResourceType type;
    uint8_t resource_id;
    uint8_t channel;
    IOPriority priority;
    uint32_t timestamp_us;
    bool active;
};

// Constants
constexpr uint8_t MAX_CHANNELS_PER_CD74HC4067 = 16;
constexpr uint8_t MAX_CHANNELS_PER_TCA9548A = 8;
constexpr uint32_t DEFAULT_ANALOG_SETTLING_TIME_US = 50;
constexpr uint32_t DEFAULT_I2C_TIMEOUT_US = 1000000; // 1 second
constexpr uint8_t INVALID_CHANNEL = 0xFF;