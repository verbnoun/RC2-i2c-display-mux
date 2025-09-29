# Raspberry Pi Pico 2 C++ Development Environment

## CRITICAL WORKFLOW RULES FOR CLAUDE CODE

### MANDATORY STARTUP AND COMPLETION PROCEDURES
When starting any Claude Code session in SoftwareC, you MUST:

1. **ON LAUNCH**: Immediately read `current.md` to understand the current project status and what we're working on
   ```bash
   # First action in any session:
   cat current.md
   ```

2. **AFTER FLASHING**: When you finish implementing and flash new code, you MUST:
   - Ask the user: "Please test the new firmware we just flashed. Does it work as expected?"
   - Wait for user confirmation before proceeding

3. **ON SUCCESS**: If the user confirms the code works:
   - Update `current.md` with the new working status
   - Create a git commit in the appropriate project repository (ValidateC/, BartlebyC/, or CandideC/):
   ```bash
   cd ValidateC  # or BartlebyC, CandideC - whichever project we're working on
   git add -A
   git commit -m "feat: [description of what now works]
   
   🤖 Generated with Claude Code
   Co-Authored-By: Claude <noreply@anthropic.com>"
   ```

### WORKFLOW SUMMARY
```
START → Read current.md → Code/Debug → Flash → Ask for test → 
  ├─ If pass: Update current.md → Git commit → Continue
  └─ If fail: Debug → Flash → Repeat
```

---

This workspace is configured for Raspberry Pi Pico 2 C++ development with custom terminal-based tools for Pico software.

## Environment Setup

**Load Environment Variables:**
```bash
source .env
```

This sets up:
- `PICO_SDK_PATH` - Official Raspberry Pi Pico SDK location
- `PICO_BOARD` - Target board (default: pico2)
- `PICO_PLATFORM` - Target platform (default: rp2350-arm-s)
- Build tool paths (cmake, ninja, arm-none-eabi-gcc)
- Custom pico-tools in PATH

## Custom Development Tools (pico-tools/bin/)

### Core Workflow Commands
- **`pico-init <project-name> [template]`** - Create new Pico project
  - **attach-part** - Professional Attach Part foundation (console logging, git versioning, command interface)
  - basic-cpp, advanced-cpp, multicore-cpp, pio-cpp - Standard pico-tools templates
- **`pico-build [target] [--clean] [--verbose]`** - Build current project
- **`pico-flash [uf2-file]`** - Flash to Pico (auto-detects device and handles BOOTSEL)
- **`pico-clean [--all] [--cache]`** - Clean build artifacts

### SDK Management Commands
- **`pico-sdk-update`** - Update SDK to latest version
- **`pico-board-set <board>`** - Switch target board (pico, pico2, etc.)
- **`pico-config-show`** - Display current configuration

## Project Repositories

### ValidateC/
**Modular library testing playground** - Primary development environment for creating and validating reusable libraries before integration into production applications. Contains hardware test rigs and example implementations for all library categories.

### BartlebyC/
**Controller Framework Application** - MIDI 2.0 controller with 25-key keyboard, potentiometers, and encoders. Uses validated libraries from ValidateC for hardware I/O, USB host, and display management.

### CandideC/
**Synthesizer Framework Application** - MIDI 2.0 synthesizer with wavetable engine, filters, and effects. Uses validated libraries from ValidateC for audio processing, USB device, and parameter management.

## Development Workflow

### 🚀 NEW: Environment Variable Architecture (NO Relative Paths)

This environment uses **environment variables** to eliminate relative path hell forever. All projects use standardized paths via `.env` variables:

- `PICO_SDK_PATH` - Pico SDK location
- `LIBRARIES_PATH` - Shared libraries (console_logger, pot_scanner, etc.)
- `PROJECT_ROOT` - SoftwareC infrastructure root

### Standard CMakeLists.txt Pattern (THE ONLY WAY)
```cmake
cmake_minimum_required(VERSION 3.13)

# Include pico-sdk via environment variable (NO relative paths)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(my_project)
pico_sdk_init()

# Add shared libraries via environment variables
add_subdirectory($ENV{LIBRARIES_PATH}/console_logger console_logger)
add_subdirectory($ENV{LIBRARIES_PATH}/pot_scanner pot_scanner)

# Rest of project...
```

### 1. Create New Project
```bash
cd ValidateC  # or BartlebyC, CandideC
pico-init my_project attach-part  # Use Attach Part professional template
cd my_project
```

### 2. Build and Flash
```bash
pico-build
pico-flash --wait  # Waits for Pico in BOOTSEL mode
```

### 3. Development Cycle
```bash
# Edit code...
pico-build --verbose
pico-flash
```

## SDK Information

- **Location:** `./pico-sdk/` (official Raspberry Pi Pico SDK)
- **Version:** Latest (cloned with all submodules)
- **Supported Boards:** pico, pico-w, pico2
- **Supported Platforms:** rp2040, rp2350-arm-s, rp2350-riscv

## Build System

- **Primary:** CMake + Ninja (fast builds)
- **Fallback:** CMake + Make
- **Toolchain:** arm-none-eabi-gcc 10.3-2021.10
- **Output Formats:** .elf (debugging), .uf2 (flashing), .hex, .bin

## Hardware Integration

### Supported Features
- GPIO, PWM, ADC, I2C, SPI, UART
- USB device/host support
- Multicore programming (dual Cortex-M33 on Pico 2)
- PIO (Programmable I/O) for custom protocols
- WiFi/Bluetooth (on Pico W variants)

### Debugging Support
- Debug probe ready (OpenOCD integration)
- Serial output via USB or UART
- Hardware breakpoints and real-time debugging

## Claude Code Integration

When working in this environment, Claude Code should:

1. **Auto-detect project type** by checking for CMakeLists.txt and pico_sdk_import.cmake
2. **Use custom build commands** instead of generic ones
3. **Provide Pico-specific assistance** for hardware APIs and SDK usage
4. **Suggest appropriate templates** when creating new projects
5. **Handle build errors** with Pico-specific troubleshooting
6. **Recommend hardware-specific solutions** for GPIO, peripherals, etc.

## Quick Commands for Claude

### Project Creation
```bash
# Create Attach Part project (RECOMMENDED)
pico-init my_controller attach-part

# Create basic C++ project (minimal)
pico-init sensor_reader basic-cpp

# Create advanced project with peripherals (complex)
pico-init robot_controller advanced-cpp
```

### Build and Deploy
```bash
# Quick build and flash
pico-build && pico-flash

# Clean rebuild
pico-clean --all && pico-build && pico-flash
```

### Troubleshooting
```bash
# Show current configuration
pico-config-show

# Verbose build for debugging
pico-build --verbose

# Update SDK if needed
pico-sdk-update
```

## Repository Architecture

### SoftwareC Infrastructure Repository (THIS REPO)
```
SoftwareC/              # Main infrastructure repository
├── .gitignore          # Ignores project directories AND library repos
├── .env                # Environment variables (THE foundation)
├── CLAUDE.md           # This documentation
├── frameworks.md       # Framework architecture
├── current.md          # Current development status
├── libraries/          # Shared library ecosystem (separate git repos)
│   ├── console_logger/ # Individual git repo - standardized logging
│   ├── pot_scanner/    # Individual git repo - CD74HC4067 + pot management
│   └── activity_led/   # Individual git repo - LED management
├── pico-sdk/           # Official Raspberry Pi Pico SDK
└── pico-tools/         # Custom development tools
    ├── bin/            # Build/flash tools + git-status-check
    ├── templates/      # Project templates (env-var based)
    └── docs/           # Tool documentation
```

### Project Repositories (Separate, Reference SoftwareC)
```
ValidateC/              # Library testing playground
BartlebyC/              # Controller application  
CandideC/               # Synthesizer application
```

### Library Development Workflow
Each library in `libraries/` is a separate git repository:
- **Individual Commits**: Each library has independent version history
- **Modular Development**: Libraries can be updated without affecting infrastructure
- **Environment Variables**: Projects still reference via `$LIBRARIES_PATH`
- **Git Status Monitoring**: Use `git-status-check` to see all repo statuses

### Multi-Repository Management
```bash
# Check all repositories for uncommitted changes
git-status-check

# Commit to specific library
cd libraries/console_logger
git add . && git commit -m "fix: improve tag colors"

# Commit to infrastructure
git add . && git commit -m "feat: add new build tool"
```

**Key Point**: Projects reference SoftwareC via environment variables. Each library is independently versioned. NO relative paths anywhere!

This environment provides professional-grade Pico 2 development capabilities entirely through terminal commands.

## Build & Flash Tool Reference

### Core Build Methods
```bash
# Method 1: Direct build system (fastest)
cd build && ninja

# Method 2: Convenience wrapper (preferred)
pico-build [--clean] [--verbose]

# Method 3: Fallback for legacy systems
cd build && make
```

### Flash Methods
```bash
# Method 1: Smart flash with auto-detection (recommended)
pico-flash

# Method 2: Full path for reliability (Claude Code preferred)
"/Users/jackson/Documents/Projects/Attach Part/Product/SoftwareC/pico-tools/bin/pico-flash" pot_reader.uf2

# Method 3: Alternative smart flash command
pico-flash-smart
```

### Environment Diagnostics
```bash
# Check configuration and paths
pico-config-show

# Clean build artifacts
pico-clean [--all] [--cache]
```

## Claude Code Quick Commands

### Standard Development Workflow
```bash
# Build and flash sequence
cd /path/to/project/build
ninja
cd ..
pico-flash

# Alternative using convenience wrapper
pico-build && pico-flash
```

### Troubleshooting Sequence
```bash
# 1. Verify environment
pico-config-show

# 2. Clean rebuild if issues
pico-clean --all
pico-build --verbose

# 3. Manual flash if auto-detection fails
pico-flash -m  # Manual BOOTSEL mode
```

### Path Requirements
- **Build Directory**: Always use `build/` subdirectory
- **Tool Paths**: pico-tools are in PATH via .env sourcing
- **Working Directory**: Commands expect project root location

## Development Best Practices - Error Prevention

### **Critical Workflow Habits**
```bash
# ALWAYS start debugging/navigation with:
pwd                           # Confirm current location
ls -la                       # Check local contents  
ls -la ../pico-tools/bin/    # Verify tool locations before use
```

### **C++ Code Requirements**
- **Include Headers**: Always add `<stdio.h>` for printf, `<algorithm>` for std::min/max
- **Build Dependencies**: Update CMakeLists.txt immediately when adding/removing source files
- **Read Before Edit**: System requires Read tool before Edit tool usage

### **Path Navigation Rules**
- **Verify Before Execute**: Use `ls` to confirm relative paths exist
- **Use Absolute Paths**: When in doubt, use full paths instead of relative
- **Directory Awareness**: Always know if you're in project root vs subdirectory

### **Incremental Development**
- **Build Early & Often**: Test compilation after each major component
- **Fix Immediately**: Don't accumulate multiple errors before testing
- **Validate Environment**: Run `pico-config-show` if build issues persist

### **Common Error Patterns to Avoid**
- Wrong relative paths (`../../` confusion)
- Missing includes in C++ files
- Outdated CMakeLists.txt after file changes
- Editing files without reading them first
- Complex changes without intermediate testing