# PROJECT_NAME

Basic Raspberry Pi Pico C++ project template.

## Features

- LED blinking on the built-in LED
- USB serial output
- Basic GPIO control
- Simple main loop structure

## Hardware Requirements

- Raspberry Pi Pico, Pico W, or Pico 2
- USB cable for programming and power

## Building

```bash
pico-build
```

## Flashing

1. Hold BOOTSEL button and connect USB (or press reset while holding BOOTSEL)
2. Run: `pico-flash --wait`

## Usage

Once flashed, the program will:
- Blink the onboard LED every 500ms
- Print "Hello, Pico!" messages via USB serial

Connect to USB serial (115200 baud) to see output:
```bash
screen /dev/ttyACM0 115200  # Linux/macOS
```

## Code Structure

- `main.cpp` - Main application code
- `CMakeLists.txt` - Build configuration
- `pico_sdk_import.cmake` - SDK integration (auto-copied)

## Customization

This template provides a foundation for:
- GPIO control and input/output
- Timer-based operations
- USB communication
- Basic Pico SDK usage patterns

## Next Steps

- Add sensor reading
- Implement button input
- Add more peripherals (I2C, SPI, PWM)
- Consider upgrading to advanced-cpp template for more features