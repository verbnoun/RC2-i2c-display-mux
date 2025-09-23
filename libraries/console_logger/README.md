# Console Logger Library

A lightweight, standardized logging library for Pico 2 C++ projects, designed for both Controller and Synthesizer frameworks.

## Features

- **Colored Output**: Automatic color coding by log level
- **Log Levels**: DEBUG, INFO, WARN, ERROR with configurable filtering
- **Standardized Tags**: Predefined tags for common components (HW, MIDI, OLED, etc.)
- **Memory Efficient**: Minimal RAM usage, configurable features
- **Fast**: Optimized for real-time embedded use

## Usage

### Basic Setup
```cpp
#include "console_logger.h"

int main() {
    // Initialize with INFO level, colors enabled
    ConsoleLogger::init(LOG_LEVEL_INFO, true, false);
    
    ConsoleLogger::banner("ValidateC Controller Framework");
    
    LOG_BOOT_INFO("System starting...");
    LOG_HW_INFO("GPIO initialized");
    LOG_WARN(TAG_SYSTEM, "Low memory warning");
    
    return 0;
}
```

### Available Tags
- `TAG_CONTROLLER`, `TAG_SYNTH` - Framework identification
- `TAG_HW`, `TAG_MIDI`, `TAG_AUDIO` - Core systems
- `TAG_OLED`, `TAG_POT`, `TAG_MUX`, `TAG_I2C`, `TAG_USB` - Hardware components
- `TAG_ANIM`, `TAG_BOOT`, `TAG_SYSTEM` - Other subsystems

### Convenience Macros
```cpp
LOG_DEBUG(TAG_HW, "ADC reading: %d", value);
LOG_INFO(TAG_MIDI, "Note on: %d velocity: %d", note, velocity);
LOG_WARN(TAG_OLED, "Display %d not responding", display_id);
LOG_ERROR(TAG_SYSTEM, "Critical failure in %s", component);

// Quick hardware logging
LOG_HW_INFO("Sensors initialized");
LOG_BOOT_ERROR("Failed to start");
```

### Output Example
```
============================================================
==                ValidateC Controller                  ==  
============================================================
[INFO ] BOOT     System starting...
[INFO ] HW       GPIO pins configured
[WARN ] SYSTEM   Low memory: 45KB free
[ERROR] OLED     Display 0 initialization failed
```

## Configuration

- **Log Level**: Set minimum level to output (DEBUG < INFO < WARN < ERROR)
- **Colors**: Enable/disable colored output
- **Timestamps**: Optional timestamps (disabled by default to save memory)

## Memory Usage

- **Flash**: ~2-3KB
- **RAM**: ~200 bytes + message buffers
- **Stack**: ~300 bytes per log call