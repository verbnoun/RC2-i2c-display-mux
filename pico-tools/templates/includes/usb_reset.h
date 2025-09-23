#ifndef USB_RESET_H
#define USB_RESET_H

#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"

// USB CDC commands for reset
#define RESET_BOOTSEL_ACTIVITY_LED 25
#define RESET_MAGIC_TOKEN 0xCAFEF00D

// Check for special baud rate to trigger BOOTSEL mode
// Common trigger: 1200 baud (Arduino-style reset)
static inline void check_usb_reset_bootsel() {
    // This is called from your main loop
    static uint32_t last_check = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    // Check every 100ms
    if (now - last_check < 100) return;
    last_check = now;
    
    // Check for magic pattern in memory (set by USB handler)
    if (watchdog_hw->scratch[5] == RESET_MAGIC_TOKEN) {
        // Clear the token
        watchdog_hw->scratch[5] = 0;
        
        // Visual feedback if LED available
        #ifdef PICO_DEFAULT_LED_PIN
        gpio_init(PICO_DEFAULT_LED_PIN);
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
        for (int i = 0; i < 3; i++) {
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            sleep_ms(100);
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            sleep_ms(100);
        }
        #endif
        
        // Reset to BOOTSEL mode
        reset_usb_boot(0, 0);
    }
}

// Alternative: Direct reset functions
static inline void reset_to_bootsel() {
    reset_usb_boot(0, 0);  // Reboot to BOOTSEL
}

static inline void reset_normal() {
    watchdog_enable(1, 1);  // Trigger watchdog reset
    while(1);
}

#endif // USB_RESET_H