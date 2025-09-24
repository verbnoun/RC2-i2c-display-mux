# SH1107 OLED Graphics Library for Raspberry Pi Pico

A modular, high-performance graphics library for SH1107 128x128 OLED displays, based on Adafruit GFX architecture and optimized for Raspberry Pi Pico SDK.

## Features

- ✅ **5x faster performance** than u8g2 library
- ✅ **Modular GPIO configuration** - customize pins for your project  
- ✅ **Rich graphics functions** - shapes, text, bitmaps, fonts
- ✅ **Multi-display support** - scale to 5+ displays with CS multiplexing
- ✅ **Non-blocking SPI** - suitable for real-time applications (MIDI controllers)
- ✅ **Proven initialization** - includes working DC-DC converter fix for SH1107

## Hardware Support

- **Display Controller**: SH1107 (128x128 pixels)
- **Communication**: SPI (4-wire + reset)
- **Speed**: Up to 10MHz SPI (configurable)
- **Displays**: 1-8 displays via CS pin multiplexing

## Quick Start

### 1. Add to your CMakeLists.txt

```cmake
# Add the graphics library
add_subdirectory($ENV{LIBRARIES_PATH}/oled_sh1107_gfx oled_sh1107_gfx)

# Link to your project
target_link_libraries(your_project
    pico_stdlib
    hardware_spi
    hardware_gpio
    oled_sh1107_gfx
)
```

### 2. Basic Usage

```cpp
#include "Adafruit_SH1107.h"

// Configure GPIO pins (customize for your setup)
SH1107_Config config = SH1107_Configs::single_display(
    2,   // SCK pin
    3,   // MOSI pin  
    4,   // DC pin
    5,   // RST pin
    6    // CS pin
);

// Create display object
Adafruit_SH1107 display(config);

// Initialize
if (!display.begin()) {
    printf("Display init failed!\n");
    return -1;
}

// Draw graphics
display.clearDisplay();
display.setTextSize(1);
display.setTextColor(SH110X_WHITE);
display.setCursor(10, 10);
display.println("Hello World!");
display.drawCircle(64, 64, 30, SH110X_WHITE);
display.display();  // Push to screen
```

### 3. Multi-Display Setup (MIDI Controllers)

```cpp
// Configure 5 displays with individual CS pins
SH1107_Config config = SH1107_Configs::midi_controller_5x(
    2,   // SCK pin (shared)
    3,   // MOSI pin (shared)
    4,   // DC pin (shared) 
    5,   // RST pin (shared)
    6, 7, 8, 9, 10  // Individual CS pins
);

Adafruit_SH1107 display(config);
display.begin();

// Draw to specific display
display.selectDisplay(0);  // Select first display
display.clearDisplay();
display.print("Display 1");
display.display();

display.selectDisplay(1);  // Select second display  
display.clearDisplay();
display.print("Display 2");
display.display();
```

## API Reference

### Configuration

```cpp
// Single display
SH1107_Config config = SH1107_Configs::single_display(sck, mosi, dc, rst, cs);

// Multi-display (up to 8)
SH1107_Config config = SH1107_Configs::midi_controller_5x(sck, mosi, dc, rst, cs1, cs2, cs3, cs4, cs5);

// Custom configuration
SH1107_Config config;
config.spi_sck_pin = 2;
config.spi_mosi_pin = 3;
config.spi_frequency = 10000000;  // 10MHz
// ... set other parameters
```

### Drawing Functions

```cpp
// Basic shapes
display.drawPixel(x, y, color);
display.drawLine(x0, y0, x1, y1, color);
display.drawRect(x, y, w, h, color);
display.fillRect(x, y, w, h, color);
display.drawCircle(x, y, r, color);
display.fillCircle(x, y, r, color);

// Text rendering
display.setTextSize(1);           // 1-4
display.setTextColor(SH110X_WHITE);
display.setCursor(x, y);
display.print("Text");
display.println("Line");

// Bitmaps
display.drawBitmap(x, y, bitmap, w, h, color);

// Display control
display.clearDisplay();          // Clear buffer
display.display();              // Push buffer to screen
display.invertDisplay(true);    // Invert colors
display.setContrast(255);       // Set brightness
```

### Colors

- `SH110X_BLACK` - Turn pixels off
- `SH110X_WHITE` - Turn pixels on  
- `SH110X_INVERSE` - Invert pixels

### Multi-Display

```cpp
display.selectDisplay(index);    // Select active display (0-7)
display.getCurrentDisplay();     // Get current display index
display.getDisplayCount();       // Get total display count

// Multi-display manager (optional)
SH110X_MultiDisplay manager(displays, count);
manager.clearAll();              // Clear all displays
manager.displayAll();            // Update all displays
manager.invertAll(true);         // Invert all displays
```

## Performance

- **Full screen update**: ~5ms at 10MHz SPI
- **Text rendering**: ~1ms per character
- **Shape drawing**: Hardware-accelerated via optimized algorithms
- **Memory usage**: 2048 bytes per 128x128 display buffer

## Examples

See `examples/` directory:
- `basic_test.cpp` - Complete demo with graphics, text, performance test

## Wiring

Standard 5-pin SPI connection:

```
SH1107 Display    Raspberry Pi Pico
--------------    -----------------
VCC            -> 3V3
GND            -> GND  
SCL (SCK)      -> GP2 (configurable)
SDA (MOSI)     -> GP3 (configurable)
DC             -> GP4 (configurable)
RST            -> GP5 (configurable)
CS             -> GP6 (configurable)
```

For multiple displays, share SCK/MOSI/DC/RST pins and use individual CS pins.

## Architecture

This library is built on proven Adafruit GFX architecture with optimizations for Pico SDK:

- `Adafruit_GFX`: Core graphics primitives and text rendering
- `Adafruit_SH110X`: Base SPI communication and display management  
- `Adafruit_SH1107`: SH1107-specific initialization and page addressing
- `SH1107_Config`: Modular configuration system

## Contributing

This library is part of the SoftwareC MIDI controller framework. Contributions welcome via standard Git workflow.

## License

Based on Adafruit GFX Library (BSD License). Pico SDK port and optimizations released under same terms.