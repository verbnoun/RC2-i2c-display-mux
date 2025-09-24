## Application Frameworks
*Status: Clean slate - Starting fresh with direct hardware control for OLED management.*

### Controller Framework
**Purpose**: Hardware input device that sends commands/data to other devices

```
Controller Framework
├── Core Lifecycle
│   ├── init() - Setup sequence
│   ├── loop() - Main event loop
│   └── shutdown() - Cleanup
│
├── Input Pipeline
│   ├── [LIBRARY: pot_scanner] ✅ - 16-channel potentiometer scanning (basic ADC)
│   ├── [LIBRARY: keyboard_2d] 🔄 - 25-key with pressure (Z) and pitch bend (X) [planned]
│   ├── [LIBRARY: encoder] 🔄 - Rotary encoder with button [planned]
│   └── process_inputs() - Transform raw → MIDI 2.0 messages [planned]
│
├── Output Pipeline  
│   ├── [LIBRARY: usb_device] - Built-in USB device
│   ├── [LIBRARY: pio_usb_host] - PIO USB host to Synth
│   ├── [LIBRARY: midi2_ump] - Universal MIDI Packets
│   ├── [LIBRARY: midi2_ci] - Capability Inquiry
│   └── send_messages() - Route MIDI 2.0 to appropriate port
│
└── System Services
    ├── [LIBRARY: console_logger] ✅ - Tag-based logging with unique colors
    ├── [LIBRARY: state_persist] 🔄 - Save/load settings [planned]
    ├── [LIBRARY: boot_manager] 🔄 - Startup sequence [planned]
    ├── [LIBRARY: oled_sh1107_spi] ✅ - SPI multi-OLED display (128x128, working)
    ├── [LIBRARY: animation_engine] 🔄 - Display animations [planned]
    └── [LIBRARY: performance_monitor] 🔄 - CPU/memory tracking [planned]
```

### Synthesizer Framework
**Purpose**: Receives commands and generates audio/responses

```
Synthesizer Framework
├── Core Lifecycle
│   ├── init() - Setup sequence
│   ├── loop() - Main processing loop
│   └── shutdown() - Cleanup
│
├── Input Pipeline
│   ├── [LIBRARY: usb_device] - Built-in USB device
│   ├── [LIBRARY: pio_usb_device] - PIO USB device from Controller
│   ├── [LIBRARY: midi2_ump] - Universal MIDI Packets
│   ├── [LIBRARY: midi2_voice] - Note with velocity, pressure, pitch bend
│   └── handle_messages() - Route to appropriate processors
│
├── Processing Core
│   ├── [LIBRARY: wavetable_synth] - Oscillator engine
│   ├── [LIBRARY: filter_bank] - DSP filters
│   ├── [LIBRARY: effects_rack] - Reverb, delay, chorus
│   ├── [LIBRARY: parameter_router] - MIDI 2.0 PE-based routing
│   └── process_audio() - Real-time audio loop
│
├── Output Pipeline
│   ├── [LIBRARY: audio_i2s_pcm5102] - PCM5102 DAC driver
│   ├── [LIBRARY: audio_daisy] - Daisy Seed (future)
│   └── output_audio() - Stream to DAC
│
└── System Services
    ├── [LIBRARY: state_persist] - Save presets
    ├── [LIBRARY: boot_manager] - Startup sequence
    ├── [LIBRARY: midi2_pe] - Property Exchange for control mapping
    ├── [LIBRARY: oled_sh1107_spi] - Optional parameter display
    └── [LIBRARY: performance_monitor] - Audio performance stats
```

## Library Categories

### Hardware I/O Libraries
- **pot_scanner** ✅ - Basic ADC potentiometer reading with EMA filtering
- **console_logger** ✅ - Tag-based logging with unique colors, system info utilities
- **oled_sh1107_spi** ✅ - SPI multi-OLED display driver (128x128, non-blocking)
- **activity_led** ✅ - LED activity patterns (deprecated in favor of inline logic)
- **keyboard_2d** 🔄 - 25-key velostat with continuous pressure (Z) and pitch bend (X) [planned]
- **encoder** 🔄 - Rotary encoder with button [planned]

### USB Libraries (Role-specific)
- **usb_device** - Built-in USB device mode (both Controller & Synth)
- **pio_usb_host** - PIO USB host for Controller only
- **pio_usb_device** - PIO USB device for Synth only

### MIDI 2.0 Libraries
- **midi2_ump** - Universal MIDI Packet core
- **midi2_ci** - Capability Inquiry (Property Exchange, Profile Config)
- **midi2_pe** - Property Exchange for control mapping
- **midi2_voice** - Note On/Off with velocity, pressure, pitch bend per-note
- **midi2_mpe_plus** - Enhanced MPE using MIDI 2.0 per-note controllers

### Display Libraries (Type-specific)
- **oled_sh1107_spi** ✅ - SPI + multiple SH1107 128x128 OLEDs (working)
- **tft_st7789** - TFT display driver (example for future)
- **animation_engine** - Shared animation framework
- **font_renderer** - Shared text rendering

### Audio Libraries (Synth)
- **audio_i2s_pcm5102** - PCM5102 DAC driver via I2S
- **audio_daisy** - Daisy Seed audio interface (future)
- **wavetable_synth** - Oscillator engine
- **filter_bank** - DSP filters
- **effects_rack** - Reverb, delay, chorus

### System Libraries
- **boot_manager** - Startup sequence with progress display
- **state_persist** - Flash/NVM storage for settings
- **parameter_router** - MIDI 2.0 Property Exchange-based routing
- **performance_monitor** - CPU/memory/audio performance tracking

## Library Development Workflow

### Structure (Current Implementation)
```
SoftwareC/              # Infrastructure repository
├── libraries/          # Organization folder (each is separate git repo)
│   ├── console_logger/ # Individual git repository
│   ├── pot_scanner/    # Individual git repository  
│   └── activity_led/   # Individual git repository
├── pico-tools/bin/git-status-check  # Multi-repo monitoring
└── .env               # LIBRARIES_PATH environment variable

ValidateC/             # Project repositories use environment variables
├── CMakeLists.txt     # add_subdirectory($ENV{LIBRARIES_PATH}/console_logger console_logger)
└── controller_test/

BartlebyC/            # Same environment variable architecture
└── CMakeLists.txt     # add_subdirectory($ENV{LIBRARIES_PATH}/pot_scanner pot_scanner)
```

### Multi-Repository Workflow
```bash
# Check all repositories for uncommitted changes
source .env && git-status-check

# Work on specific library
cd libraries/console_logger
git commit -m "feat: add new tag color"

# Work on infrastructure  
git commit -m "feat: add new build tool"

# Work on application
cd ValidateC
git commit -m "feat: add new controller feature"
```

### Benefits
- **Individual Library Versioning**: Each library has independent commit history and releases
- **Modular Development**: Libraries evolve independently without affecting infrastructure
- **Environment Variable Architecture**: Eliminates relative path hell (`../../` patterns)
- **Multi-Repo Monitoring**: Single `git-status-check` command shows all repository statuses
- **Mix and Match**: Libraries work across Controller and Synthesizer projects
- **Clear Separation**: Infrastructure, libraries, and applications have distinct responsibilities