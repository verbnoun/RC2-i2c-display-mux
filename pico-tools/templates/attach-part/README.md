# Attach Part Project Template

Professional foundation template for Raspberry Pi Pico 2 projects with console logging, version tracking, and command interface.

## Features

- ✅ **Console Logger Integration** - Professional tag-based logging system
- ✅ **Git Hash Version Tracking** - Automatic build version information  
- ✅ **Console Command Interface** - Interactive debugging and control
- ✅ **Boot Delay Protection** - Compatible with flash tool workflow
- ✅ **Graceful Shutdown** - Clean system restart capability
- ✅ **Watchdog Protection** - Development safety with automatic recovery
- ✅ **Hardware Agnostic** - No assumptions about external hardware

## Template Structure

```
PROJECT_NAME/
├── CMakeLists.txt          # Build configuration with console_logger
├── main.cpp                # Professional foundation template
└── README.md               # Project documentation
```

## Quick Start

1. **Create new project** using pico-init:
   ```bash
   cd ValidateC  # or your project directory
   pico-init my_new_project attach-part
   cd my_new_project
   ```

2. **Build and flash**:
   ```bash
   pico-build
   pico-flash
   ```

3. **Connect to console** and press 'h' for help

## Built-in Commands

- **h** - Show help and version information
- **r** - Restart system
- **S** - Graceful shutdown (capital S for safety)

## Development Workflow

### Adding Your Code

1. **Initialization**: Add hardware setup after "Add your initialization code here..."
2. **Main Loop**: Add your logic after "Add your main loop code here..."  
3. **Commands**: Add new console commands in `process_console_input()`
4. **Libraries**: Add new libraries to CMakeLists.txt as needed

### Example Extensions

```c
// Add to process_console_input() switch statement:
case 't':
    LOG(TAG_SYSTEM, "Running test sequence...");
    // Your test code here
    break;

case 's':
    LOG(TAG_SYSTEM, "System status: OK");
    // Your status display here
    break;
```

### Adding Hardware Libraries

```cmake
# In CMakeLists.txt, add libraries as needed:
add_subdirectory($ENV{LIBRARIES_PATH}/i2c_display_mux i2c_display_mux)
add_subdirectory($ENV{LIBRARIES_PATH}/ssd1306_graphics ssd1306_graphics)

# Add to target_link_libraries:
target_link_libraries(PROJECT_NAME
    # ... existing libraries ...
    i2c_display_mux
    ssd1306_graphics
)

# Add required hardware libraries:
target_link_libraries(PROJECT_NAME
    # ... existing libraries ...
    hardware_i2c        # For I2C displays
    hardware_spi        # For SPI devices
    hardware_adc        # For analog sensors
)
```

## Features Included

### Professional Logging
- Color-coded console output with timestamps
- Tag-based logging (TAG_SYSTEM, TAG_HW, etc.)
- Multiple log levels (DEBUG, INFO, WARN, ERROR)
- Banner display with project information

### Version Tracking
- Automatic git hash integration
- Build timestamp tracking
- Version display in help command

### Development Safety
- Boot delay prevents flash tool interference
- Watchdog protection with 8-second timeout  
- Graceful shutdown with cleanup hooks
- Console attachment time during startup

### Interactive Console
- Real-time command processing
- Help system with version information
- Safe restart and shutdown commands
- Easy to extend with custom commands

## Best Practices

1. **Use LOG() instead of printf()** - Better formatting and control
2. **Add commands for testing** - Interactive debugging is invaluable
3. **Include status commands** - Easy system health checking
4. **Keep main loop fast** - 1ms sleep minimum for responsiveness
5. **Use descriptive log tags** - Makes debugging much easier

## Attach Part Standards

This template follows Attach Part development standards:
- Environment variable-based library paths (no relative path hell)
- Console logger for all output (no raw printf)
- Professional version tracking and help systems
- Hardware-agnostic foundation with library extensions
- Safe development practices with watchdog and graceful shutdown

## Next Steps

After creating your project:
1. Update PROJECT_NAME in CMakeLists.txt and main.cpp
2. Add your specific hardware libraries as needed
3. Implement your application logic in the marked sections
4. Add custom console commands for debugging
5. Test with clean restart/shutdown cycle