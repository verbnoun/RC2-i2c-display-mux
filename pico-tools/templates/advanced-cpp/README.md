# PROJECT_NAME

Advanced Raspberry Pi Pico C++ project template with peripheral support.

## Features

- **GPIO Control**: LED, button input with debouncing
- **PWM**: Variable brightness control
- **ADC**: Analog sensor reading (temperature, light)
- **I2C**: Sensor communication and device scanning
- **SPI**: High-speed sensor interface
- **Display**: OLED display support (SSD1306 example)
- **Watchdog**: System reliability monitoring
- **Unique ID**: Board identification
- **USB Serial**: Rich status output and debugging

## Hardware Requirements

- Raspberry Pi Pico, Pico W, or Pico 2
- Optional: I2C sensors (BME280, etc.)
- Optional: SPI devices
- Optional: OLED display (SSD1306)
- Optional: Push button on GPIO 2
- Optional: Light sensor on ADC0 (GPIO 26)

## Pin Configuration

### Default Pins:
- **LED**: Built-in LED pin
- **Button**: GPIO 2 (with internal pull-up)
- **PWM Output**: GPIO 15
- **ADC Input**: GPIO 26 (ADC0)

### I2C (Sensors):
- **SDA**: GPIO 4
- **SCL**: GPIO 5

### I2C (Display):
- **SDA**: GPIO 6
- **SCL**: GPIO 7

### SPI:
- **MISO**: GPIO 16
- **MOSI**: GPIO 19
- **SCK**: GPIO 18
- **CS**: GPIO 17

## Building

```bash
pico-build
```

## Flashing

```bash
pico-flash --wait
```

## Usage

The program demonstrates:

1. **Sensor Reading**: Temperature and light level monitoring
2. **Interactive Control**: Button changes LED blink rate
3. **Automatic Brightness**: PWM brightness based on light sensor
4. **I2C Communication**: Device scanning and sensor communication
5. **Display Updates**: Real-time data display (if connected)
6. **System Monitoring**: Watchdog and uptime tracking

## Serial Output

Connect to USB serial (115200 baud) to see:
- System initialization messages
- Periodic status updates
- Sensor readings
- I2C device scan results
- Display update notifications

## Code Structure

- `main.cpp` - Main application and system coordination
- `sensor.h/cpp` - I2C and SPI sensor interfaces
- `display.h/cpp` - Display management and rendering
- `CMakeLists.txt` - Build configuration with all peripherals

## Customization

This template provides examples for:
- **Adding Sensors**: Extend sensor.cpp with specific device drivers
- **Display Graphics**: Implement text and graphics rendering
- **Communication**: Add UART, WiFi, or Bluetooth capabilities
- **Real-time Control**: Implement PID controllers or state machines
- **Data Logging**: Store sensor data or configuration

## Peripheral Integration

### I2C Sensors
Modify `sensor_read_temperature()` and `sensor_read_humidity()` with your specific sensor's protocol.

### SPI Devices
Use `spi_sensor_read()` as a template for SPI communication.

### Display
The display module supports SSD1306 OLED displays but can be adapted for other I2C displays.

## Error Handling

- Graceful fallback when peripherals are not connected
- Watchdog protection against system hangs
- I2C timeout and error checking
- Safe GPIO initialization

## Performance

- Non-blocking sensor updates
- Efficient peripheral management
- Minimal memory footprint
- Watchdog-protected main loop

## Next Steps

- Add specific sensor drivers for your application
- Implement data storage or transmission
- Add user interface elements
- Consider upgrading to multicore-cpp for parallel processing