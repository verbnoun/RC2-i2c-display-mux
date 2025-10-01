//============================================================================
// PROJECT_NAME - Attach Part Foundation Template
// Professional development foundation with console logging and command interface
//============================================================================

// Version Information (will be set by build process)
#ifndef GIT_HASH
    #define GIT_HASH "unknown"
#endif
#ifndef BUILD_DATE
    #define BUILD_DATE __DATE__ " " __TIME__
#endif
#define PROJECT_NAME "PROJECT_NAME"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"

// Our custom libraries
#include "console_logger.h"

//============================================================================
// CONFIGURATION
//============================================================================
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
// NOTE: No boot delay needed! SDK's PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS handles USB timing

//============================================================================
// CONSOLE INTERFACE
//============================================================================
void show_help() {
    LOG(TAG_SYSTEM, "=== %s ===", PROJECT_NAME);
    LOG(TAG_SYSTEM, "Git Hash: %s", GIT_HASH);
    LOG(TAG_SYSTEM, "Build: %s", BUILD_DATE);
    LOG(TAG_SYSTEM, "Platform: Raspberry Pi Pico 2");
}

void process_console_input() {
    int c = getchar_timeout_us(0);
    
    if (c != PICO_ERROR_TIMEOUT) {
        switch (c) {
            case 'h':
                show_help();
                break;
                
            case 'r':
                LOG(TAG_SYSTEM, "Restarting system...");
                sleep_ms(500);
                watchdog_reboot(0, 0, 10);
                break;
                
            case 'S':  // Capital S for safety - shutdown command
                LOG(TAG_SYSTEM, "=== GRACEFUL SHUTDOWN INITIATED ===");
                LOG(TAG_SYSTEM, "Cleaning up system state...");
                
                // Add your cleanup code here
                
                LOG(TAG_SYSTEM, "✓ Cleanup complete");
                LOG(TAG_SYSTEM, "Restarting system cleanly...");
                
                // Brief delay for serial output
                sleep_ms(100);
                
                // Software reboot - clean restart, stays in normal mode
                watchdog_reboot(0, 0, 10);
                break;
                
            default:
                if (c >= 32 && c <= 126) {
                    LOG(TAG_SYSTEM, "Unknown command '%c' - press 'h' for help", c);
                }
                break;
        }
    }
}

//============================================================================
// MAIN FUNCTION
//============================================================================
int main() {
    // Initialize console logging (SDK handles USB timing via PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS)
    ConsoleLogger::init(LOG_LEVEL_INFO, true, false);
    ConsoleLogger::banner(PROJECT_NAME);
    LOG(TAG_SYSTEM, "Project: %s | Build: %s", PROJECT_NAME, BUILD_DATE);
    LOG(TAG_SYSTEM, "Git Hash: %s", GIT_HASH);

    // Initialize LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

    // System ready
    LOG(TAG_SYSTEM, "=== System Ready ===");
    LOG(TAG_SYSTEM, "Commands: h=help, r=restart, S=shutdown");
    LOG(TAG_SYSTEM, "Add your initialization code here...");

    // Enable watchdog
    watchdog_enable(8000, 1);
    
    uint32_t heartbeat_counter = 0;
    
    while (true) {
        // Process console commands
        process_console_input();
        
        // Add your main loop code here
        
        // Heartbeat every 10 seconds
        if (++heartbeat_counter >= 10000) {
            heartbeat_counter = 0;
            LOG(TAG_SYSTEM, "💓 %s running", PROJECT_NAME);
        }
        
        // Feed watchdog
        watchdog_update();
        sleep_ms(1);
    }
    
    return 0;
}