# Current Development Status
*Current state snapshot - latest changes and immediate next steps only*

## Current Focus: Keyboard Validation

Starting keyboard_2d library development for 25-key velostat keyboard with continuous pressure (Z) and pitch bend (X) per key.

### Next Tasks
- Define keyboard hardware specifications (rows, columns, pin assignments)
- Create keyboard_2d library in libraries/
- Implement keyboard scanning and pressure sensing
- Test and validate in kb_validation project

## Recent Work Completed (Today)

### System Initialization Architecture Fix (October 1, 2025)
- **Root Cause**: ConsoleLogger library was calling `stdio_init_all()` (system-level initialization)
- **Architecture Fix**: Removed `stdio_init_all()` from library, added explicit call in applications
- **Separation of Concerns**: System init (application) vs configuration (library)
- **Pattern**: Application calls `stdio_init_all()` before `ConsoleLogger::init()`
- **Template Updated**: attach-part template now includes explicit `stdio_init_all()` call
- **macOS USB Timing**: Occasional hang on rapid reflash (macOS USB enumeration timing), resolved by graceful restart
- **Libraries Updated**: console_logger
- **Projects Updated**: pot_oled_validation, kb_validation (template)

### Attach-Part Template Updates (October 1, 2025)
- **Zero-Delay Boot**: Applied pot_oled_validation learnings to template
- **USB Timing**: `PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=2000` in CMakeLists.txt
- **Explicit System Init**: Template now calls `stdio_init_all()` before ConsoleLogger
- **Removed Boot Delay**: Eliminated 1250ms delay + 3s countdown from template
- **Minimal Help Menu**: Simplified to just project name, git hash, build date, platform
- **Template Documentation**: Added changelog and update philosophy to README.md

### KB Validation Project Created (October 1, 2025)
- **New Project**: ValidateC/kb_validation/ using updated attach-part template
- **Purpose**: Validate keyboard_2d library independently before integration
- **Status**: Foundation built and flashed, ready for keyboard hardware implementation

### Development Tools (October 1, 2025)
- **git-status-check**: Now skips pico-sdk and build artifacts (_deps)
- **Multi-Repo Management**: Clean separation of infrastructure vs library vs project repos

## Recent Work Completed (Previous)

### Screensaver Architecture Refactor (September 30, 2025)
- **Per-Screen Animation**: 5 independent bouncing balls with randomized directions
- **Page System Integration**: Screensaver now behaves as normal page (no special exit logic)
- **State Preservation**: Original page timing restored on exit (fixes auto-return)
- **Event Consumption**: Triggering event consumed on exit (prevents partial restore)
- **Status Boot Display**: Status screen shows "STATUS" instead of blank on boot
- **Default Threshold**: Changed from 45 to 50 for better responsiveness

### Zero-Delay Boot Architecture (September 30, 2025)
- **Boot Optimization**: Removed 1250ms delay + 3s countdown = 4.25s faster boot
- **SDK USB Timing**: `PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=2000` handles macOS enumeration
- **Early OLED Init**: Displays light up in ~200ms (before console/USB ready)
- **BARTLEBY Splash**: Shows "BA RT LE BY" on pot OLEDs immediately on boot
- **Display-First Architecture**: `init_displays_early()` then `init_hardware()`

### Display Artifacts Fixed
- **Root Cause**: Boot text at x=0, corner clear regions started at x=2
- **Solution**: Extended clear region for left corners to cover x=0-2 boot remnants
- **Architecture Simplification**: Removed redundant `update_corner_oled()` (65 lines)
- **Dirty Region Tracking**: Now uses library's automatic dirty region system properly

### Console Auto-Reconnect
- **Auto-Reconnect Loop**: Detects disconnect vs user exit, waits for device reappear
- **Persistent Sessions**: Console stays open through flashing/reboots
- **Continuous Logging**: Appends to same log file across reconnects

### Contrast Control Working
- **Implementation**: `-` and `=` keys adjust brightness (steps of 16)
- **SDK Integration**: Contrast commands take effect immediately in hardware
- **Status Display**: Shows current brightness level on status OLED

### Controller Architecture Complete (September 29, 2025)
- **Pot Scanning**: 16-channel scanning with CD74HC4067, simple threshold detection
- **OLED Display**: Multi-display pot coordination with DisplayManager
- **Hardware Validated**: 500μs settling, 30 ADC threshold, RC filtering (470Ω + 47nF)
- **Display Mapping**: Hardware-specific pot→OLED assignments with corner layouts

### Library Status
- **CD74HC4067 Library**: Production ready with validated hardware settings
- **Pot Scanner Library**: Production ready with threshold-based change detection
- **Console Logger**: Production ready with tag-based color logging
- **Graphics Library**: Production ready with UTF-8 native API + dirty region tracking
- **Display Mux Library**: Production ready for 8-channel display management

## Hardware Configuration
- **Platform**: Raspberry Pi Pico 2 (RP2350)
- **Display**: 5x SSD1306 OLEDs via TCA9548A (GP14 SDA, GP15 SCL)
- **Pot Scanning**: 16x pots via CD74HC4067 (GP8-11 control, GP28 ADC with RC filter)

## Development Environment
- **Build Tools**: pico-tools with auto-flash support
- **Console**: Auto-reconnecting console script for persistent sessions
- **Libraries Path**: Environment variable based (no relative paths)
- **Project Structure**: ValidateC (testing), BartlebyC (controller), CandideC (synth)
- **Template System**: `pico-init PROJECT_NAME attach-part` for new projects
