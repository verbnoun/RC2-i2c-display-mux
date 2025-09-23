# USB Reset Workflow for Raspberry Pi Pico Development

Avoid unplugging and BOOTSEL button stress with smart software reset capabilities.

## 🚀 Quick Start

### The Easy Way - Software Reset
```bash
# Connect to your Pico's console
validate-console

# Press 'b' to trigger BOOTSEL mode
# The Pico will reboot as a USB drive

# In another terminal, flash your code
pico-flash-smart
```

## 📡 Available Reset Methods

### 1. Built-in Serial Command (Recommended)
Every project built with our templates now includes a 'b' command that triggers BOOTSEL mode:
- Connect via serial console
- Press 'b' 
- Pico reboots to BOOTSEL mode
- Flash your new firmware
- No physical button pressing required!

### 2. Smart Flash Script
```bash
# Attempts automatic reset, falls back to manual
pico-flash-smart

# Specify serial device
pico-flash-smart -d /dev/tty.usbmodem1101

# Use Python method (more reliable)
pico-flash-smart -p

# Manual mode only (skip auto-reset)
pico-flash-smart -m
```

### 3. Direct Reset in Your Code
Include the USB reset header in your project:
```cpp
#include "pico/bootrom.h"

// Add to your serial command handler
case 'b':
    printf("Rebooting to BOOTSEL...\n");
    sleep_ms(2000);
    reset_usb_boot(0, 0);  // Reset to BOOTSEL
    break;
```

## 🔧 Hardware Alternative - Reset Button

If software reset isn't working, add a physical reset button:

```
Pico Pin Connections:
─────────────────────
GND ─────[Button]───── RUN

Press sequence:
1. Hold RESET button
2. Press BOOTSEL button  
3. Release RESET
4. Release BOOTSEL
```

## 🎯 Best Practices

### Development Workflow
1. **Initial Flash**: Use BOOTSEL button once for first program
2. **Subsequent Updates**: Use serial 'b' command or pico-flash-smart
3. **Rapid Testing**: Keep console open, use 'b' + pico-flash-smart

### Troubleshooting

**Pico not resetting?**
- Ensure `pico_enable_stdio_usb(1)` in CMakeLists.txt
- Check serial connection is active
- Try the Python reset method: `pico-flash-smart -p`

**Can't find serial device?**
```bash
# List all USB serial devices
ls /dev/tty.usb*
ls /dev/cu.usb*
```

**Reset command not working?**
- Rebuild with latest template that includes bootrom header
- Check you have USB stdio enabled in CMakeLists.txt
- Verify serial console is connected

## 🛠️ Advanced: Custom Reset Triggers

### Baud Rate Trigger (Arduino-style)
The smart flash script can trigger reset by changing baud rate to 1200:
```python
import serial
ser = serial.Serial('/dev/tty.usbmodem1101', 1200)
ser.close()
# Pico resets to BOOTSEL
```

### Double-Tap Reset (RP2040 only)
Note: The `pico_bootsel_via_double_reset` library doesn't work on RP2350 yet due to bootrom differences.

## 📊 Comparison Table

| Method | Works on RP2350 | Requires Code Change | Reliability |
|--------|-----------------|---------------------|-------------|
| Serial 'b' command | ✅ | ✅ (once) | High |
| pico-flash-smart | ✅ | ✅ (once) | Medium |
| Physical reset button | ✅ | ❌ | High |
| BOOTSEL button | ✅ | ❌ | High |
| picotool (if installed) | ✅ | ❌ | Medium |
| Double-tap reset | ❌ | ✅ | N/A |

## 💡 Tips

1. **Save USB Port Wear**: Software reset prevents physical USB connector wear
2. **Faster Development**: No need to reach for the board
3. **Remote Development**: Flash boards that aren't physically accessible
4. **Automation**: Script continuous integration testing

## 📝 Implementation Checklist

For your projects to support USB reset:

- [ ] Include `pico/bootrom.h` header
- [ ] Add reset command to serial handler
- [ ] Enable USB stdio in CMakeLists.txt
- [ ] Test reset command works
- [ ] Document available commands

## 🔄 Update Existing Projects

To add USB reset to existing projects:

```bash
# 1. Edit main.cpp to add includes
#include "pico/bootrom.h"

# 2. Add to command handler
case 'b':
    reset_usb_boot(0, 0);
    break;

# 3. Rebuild
pico-build

# 4. Flash once with BOOTSEL
pico-flash

# 5. Future updates use software reset!
```

---

*No more USB port stress! Your Pico and USB cable will thank you. 🎉*