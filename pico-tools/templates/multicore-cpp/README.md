# PROJECT_NAME

Advanced dual-core Raspberry Pi Pico C++ project template demonstrating multicore programming.

## Features

### Core 0 (Main Core):
- **User Interface**: Button input with debouncing
- **Output Control**: LED blinking and PWM brightness
- **System Monitoring**: Core1 health monitoring and statistics
- **Status Display**: Comprehensive system status reporting
- **Inter-core Communication**: Data exchange and synchronization

### Core 1 (Secondary Core):
- **Sensor Reading**: High-frequency ADC sampling
- **Data Processing**: Real-time sensor data analysis
- **Adaptive Control**: Automatic brightness adjustment
- **Performance Monitoring**: Loop timing and statistics

### Synchronization:
- **Mutexes**: Safe data access between cores
- **Semaphores**: Event signaling and data ready notifications
- **Critical Sections**: Atomic operations for shared variables
- **Heartbeat Monitoring**: Core health and responsiveness tracking

## Hardware Requirements

- Raspberry Pi Pico 2 (recommended for dual Cortex-M33 cores)
- Optional: Push button on GPIO 2
- Optional: Light sensor on GPIO 27 (ADC1)
- Optional: LED or device on GPIO 15 (PWM output)

## Pin Configuration

### Core 0 (User Interface):
- **LED**: Built-in LED pin
- **Button**: GPIO 2 (with internal pull-up)
- **PWM Output**: GPIO 15

### Core 1 (Sensors):
- **Temperature**: Internal ADC (ADC4)
- **Light Sensor**: GPIO 27 (ADC1)

## Building

```bash
pico-build
```

## Flashing

```bash
pico-flash --wait
```

## Usage

### System Operation:

1. **Core0**: Handles user interface and system coordination
2. **Core1**: Continuously samples sensors and processes data
3. **Button Control**: Press button to cycle through sample rates (50ms/100ms/200ms/500ms) and toggle LED
4. **Automatic Control**: Core1 adjusts LED brightness based on light and temperature

### Serial Output:

Connect to USB serial (115200 baud) to see:
- Dual-core startup sequence
- Real-time sensor readings
- Inter-core synchronization status
- Performance statistics
- Health monitoring alerts

## Code Structure

- `main.cpp` - Core0 main loop and system coordination
- `core1_tasks.h/cpp` - Core1 dedicated processing tasks
- `shared_data.h/cpp` - Thread-safe inter-core communication
- `CMakeLists.txt` - Multicore build configuration

## Multicore Architecture

### Data Flow:
1. **Core1** → Reads sensors → Updates shared data
2. **Core1** → Processes data → Calculates optimal settings
3. **Core0** → Reads shared data → Updates outputs
4. **Core0** → Handles user input → Updates control parameters

### Synchronization Strategy:
- **Critical Sections**: For simple atomic updates
- **Mutexes**: For complex data structures
- **Semaphores**: For event notification
- **Volatile Variables**: For simple status flags

## Performance Features

### Core1 Optimizations:
- High-frequency sensor sampling
- Real-time data processing
- Minimal latency operations
- Performance timing measurement

### Core0 Optimizations:
- User interface responsiveness
- Non-blocking output updates
- System health monitoring
- Efficient status reporting

## Advanced Features

### Health Monitoring:
- Heartbeat counters for both cores
- Automatic stall detection
- Performance statistics tracking
- System uptime monitoring

### Adaptive Control:
- Temperature-based thermal protection
- Light-based brightness adjustment
- Dynamic sample rate control
- User preference integration

## Error Handling

- Timeout protection for inter-core communication
- Core stall detection and reporting
- Graceful degradation when sensors unavailable
- Safe shutdown procedures

## Customization

### Adding New Core1 Tasks:
```cpp
void core1_custom_task() {
    // Your high-frequency processing code
    // Update shared data with results
}
```

### Adding New Shared Data:
```cpp
// In shared_data.h
volatile float custom_value;

// In shared_data.cpp
void set_custom_data(float value) {
    critical_section_enter_blocking(&g_sync.critical_sec);
    g_shared_data.custom_value = value;
    critical_section_exit(&g_sync.critical_sec);
}
```

## Performance Guidelines

### Core1 Best Practices:
- Keep tasks deterministic and fast
- Avoid blocking operations
- Use local variables for computation
- Update shared data atomically

### Core0 Best Practices:
- Handle user interface responsively
- Don't block on Core1 operations
- Use timeouts for inter-core communication
- Provide meaningful status information

## Real-Time Considerations

- **Core1**: Designed for real-time sensor processing
- **Deterministic Timing**: Consistent loop execution
- **Priority Handling**: Critical tasks get CPU time
- **Resource Management**: Efficient memory and CPU usage

## Next Steps

- Add more sensors to Core1 processing
- Implement data logging or transmission
- Add network communication (WiFi/Bluetooth)
- Implement more complex control algorithms
- Add real-time data visualization

## Debugging Tips

- Use heartbeat counters to verify core operation
- Monitor timing statistics for performance analysis
- Check semaphore timeouts for communication issues
- Use separate printf streams for each core if needed