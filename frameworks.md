## Application Frameworks

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
│   ├── [LIBRARY: keyboard_2d] - 25-key with pressure (Z) and pitch bend (X)
│   ├── [LIBRARY: pot_scanner] - Potentiometer array via mux
│   ├── [LIBRARY: encoder] - Rotary encoder with button
│   └── process_inputs() - Transform raw → MIDI 2.0 messages
│
├── Output Pipeline  
│   ├── [LIBRARY: usb_device] - Built-in USB device
│   ├── [LIBRARY: pio_usb_host] - PIO USB host to Synth
│   ├── [LIBRARY: midi2_ump] - Universal MIDI Packets
│   ├── [LIBRARY: midi2_ci] - Capability Inquiry
│   └── send_messages() - Route MIDI 2.0 to appropriate port
│
└── System Services
    ├── [LIBRARY: state_persist] - Save/load settings
    ├── [LIBRARY: boot_manager] - Startup sequence
    ├── [LIBRARY: oled_ssd1306_mux] - Multi-OLED display
    ├── [LIBRARY: animation_engine] - Display animations
    └── [LIBRARY: performance_monitor] - CPU/memory tracking
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
    ├── [LIBRARY: oled_ssd1306_mux] - Optional parameter display
    └── [LIBRARY: performance_monitor] - Audio performance stats
```

## Library Categories

### Hardware I/O Libraries
- **keyboard_2d** - 25-key velostat with continuous pressure (Z) and pitch bend (X)
- **mux_adc** - CD74HC4067 multiplexer control
- **encoder** - Rotary encoder with button
- **pot_scanner** - Potentiometer array reading

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
- **oled_ssd1306_mux** - TCA9548A + multiple SSD1306 OLEDs
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

### Structure
```
/libraries/              # Shared library repository
├── keyboard_2d/        # Each library is a git submodule
├── midi2_ump/         
├── animation_engine/   
└── ...

/ValidateC/             # Projects reference shared libraries
├── CMakeLists.txt     # add_subdirectory(../libraries/keyboard_2d)
└── pot_reader/

/BartlebyC/            # Updates to libraries are available on rebuild
└── CMakeLists.txt     # add_subdirectory(../libraries/keyboard_2d)
```

### Benefits
- Update a library once, all projects get the update on next compile
- Version control per library with git submodules
- Mix and match libraries between Controller and Synthesizer projects
- Clear separation of concerns