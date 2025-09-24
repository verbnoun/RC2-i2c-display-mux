//============================================================================
// SH1107 OLED Graphics Library - Basic Test Example
// Demonstrates modular usage with configurable GPIO pins
//============================================================================

#include <stdio.h>
#include "pico/stdlib.h"
#include "console_logger.h"

// Include the modular SH1107 graphics library
#include "Adafruit_SH1107.h"

//============================================================================
// CONFIGURATION - Customize for your hardware setup
//============================================================================

// Single Display Configuration (modify pins as needed)
constexpr uint SPI_SCK_PIN  = 2;  // GP2 - SPI Clock
constexpr uint SPI_MOSI_PIN = 3;  // GP3 - SPI Data Out
constexpr uint SPI_DC_PIN   = 4;  // GP4 - Data/Command Select
constexpr uint SPI_RST_PIN  = 5;  // GP5 - Reset
constexpr uint SPI_CS_PIN   = 6;  // GP6 - Chip Select

// For MIDI controller with 5 displays, uncomment these:
// constexpr uint SPI_CS_PINS[5] = {6, 7, 8, 9, 10};  // GP6-GP10

//============================================================================
// GLOBAL VARIABLES
//============================================================================

// Create display object with configuration
Adafruit_SH1107* display = nullptr;

//============================================================================
// GRAPHICS DEMO FUNCTIONS
//============================================================================

void demo_basic_drawing() {
    LOG(TAG_SYSTEM, "Running basic drawing demo...");
    
    display->clearDisplay();
    
    // Draw some shapes
    display->drawRect(10, 10, 50, 30, SH110X_WHITE);
    display->fillRect(70, 10, 30, 30, SH110X_WHITE);
    display->drawCircle(32, 70, 20, SH110X_WHITE);
    display->fillCircle(96, 70, 15, SH110X_WHITE);
    display->drawLine(0, 0, 127, 127, SH110X_WHITE);
    
    display->display();
    sleep_ms(3000);
}

void demo_text_rendering() {
    LOG(TAG_SYSTEM, "Running text rendering demo...");
    
    display->clearDisplay();
    
    // Basic text
    display->setTextSize(1);
    display->setTextColor(SH110X_WHITE);
    display->setCursor(0, 0);
    display->println("Hello SH1107!");
    
    display->setCursor(0, 16);
    display->println("128x128 OLED");
    
    // Larger text
    display->setTextSize(2);
    display->setCursor(0, 40);
    display->println("BIG TEXT");
    
    // Different sizes
    display->setTextSize(1);
    display->setCursor(0, 70);
    display->print("Size 1: ");
    display->setTextSize(2);
    display->print("2");
    
    display->display();
    sleep_ms(3000);
}

void demo_bitmap_pattern() {
    LOG(TAG_SYSTEM, "Running bitmap pattern demo...");
    
    display->clearDisplay();
    
    // Create a simple checkerboard pattern
    for (int y = 0; y < 128; y += 8) {
        for (int x = 0; x < 128; x += 8) {
            if ((x/8 + y/8) % 2) {
                display->fillRect(x, y, 8, 8, SH110X_WHITE);
            }
        }
    }
    
    display->display();
    sleep_ms(2000);
}

void demo_performance_test() {
    LOG(TAG_SYSTEM, "Running performance test...");
    
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    
    for (int i = 0; i < 10; i++) {
        display->clearDisplay();
        display->fillScreen(SH110X_WHITE);
        display->display();
        
        display->clearDisplay();
        display->display();
    }
    
    uint32_t end_time = to_ms_since_boot(get_absolute_time());
    uint32_t total_time = end_time - start_time;
    
    LOG(TAG_SYSTEM, "Performance: 20 full-screen updates in %dms (%.1f fps)", 
        total_time, 20000.0f / total_time);
}

//============================================================================
// MAIN PROGRAM
//============================================================================

int main() {
    // Boot delay
    sleep_ms(1250);
    
    // Initialize console logging
    ConsoleLogger::init(LOG_LEVEL_INFO, true, false);
    ConsoleLogger::enableTag(TAG_HW);
    ConsoleLogger::enableTag(TAG_SYSTEM);
    
    ConsoleLogger::banner("SH1107 Graphics Library Test");
    LOG(TAG_SYSTEM, "Starting modular OLED graphics demo...");
    
    // Create display configuration
    SH1107_Config config = SH1107_Configs::single_display(
        SPI_SCK_PIN, SPI_MOSI_PIN, SPI_DC_PIN, SPI_RST_PIN, SPI_CS_PIN
    );
    
    // Alternative configuration for multi-display setup:
    // SH1107_Config config = SH1107_Configs::midi_controller_5x(
    //     SPI_SCK_PIN, SPI_MOSI_PIN, SPI_DC_PIN, SPI_RST_PIN,
    //     SPI_CS_PINS[0], SPI_CS_PINS[1], SPI_CS_PINS[2], SPI_CS_PINS[3], SPI_CS_PINS[4]
    // );
    
    // Create display object
    display = new Adafruit_SH1107(config);
    
    // Initialize display
    if (!display->begin()) {
        LOG(TAG_SYSTEM, "❌ Failed to initialize SH1107 display!");
        return -1;
    }
    
    LOG(TAG_SYSTEM, "✅ SH1107 display initialized successfully");
    LOG(TAG_SYSTEM, "Display size: %dx%d pixels", display->width(), display->height());
    
    // Run demo sequence
    while (true) {
        demo_basic_drawing();
        demo_text_rendering();
        demo_bitmap_pattern();
        demo_performance_test();
        
        LOG(TAG_SYSTEM, "Demo cycle complete. Restarting...");
        sleep_ms(1000);
    }
    
    return 0;
}

//============================================================================
// USAGE INSTRUCTIONS
//============================================================================

/*

TO USE THIS EXAMPLE:

1. Update your project's CMakeLists.txt:
   
   # Add the graphics library
   add_subdirectory($ENV{LIBRARIES_PATH}/oled_sh1107_gfx oled_sh1107_gfx)
   add_subdirectory($ENV{LIBRARIES_PATH}/console_logger console_logger)
   
   # Link libraries to your project
   target_link_libraries(your_project
       pico_stdlib
       hardware_spi
       hardware_gpio
       oled_sh1107_gfx
       console_logger
   )

2. Customize GPIO pins in the configuration section above

3. For multiple displays:
   - Uncomment the SPI_CS_PINS array
   - Uncomment the midi_controller_5x configuration
   - Use display->selectDisplay(index) to switch between displays

4. Build and flash:
   mkdir build && cd build
   cmake .. && make
   cp your_project.uf2 /Volumes/RP2350

KEY FEATURES DEMONSTRATED:
- Configurable GPIO pins
- Rich graphics functions (shapes, text, bitmaps)
- Performance testing
- Modular library usage
- Multi-display support (when uncommented)

*/