#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

// Boot safety configuration
const uint32_t BOOT_DELAY_MS = 1250;  // Prevents flash tool interference
const uint32_t COUNTDOWN_SECONDS = 3;  // Console attachment time

// Simple countdown to allow console attachment after flash
void startup_countdown() {
    printf("\n=== Starting %d second countdown ===\n", COUNTDOWN_SECONDS);
    printf("LED blinks faster as countdown approaches zero...\n\n");
    
    for (uint32_t seconds_left = COUNTDOWN_SECONDS; seconds_left > 0; seconds_left--) {
        printf("Starting in %d...\n", seconds_left);
        
        // Blink rate increases as countdown approaches zero
        uint32_t blink_period_ms = 100 * seconds_left;
        uint32_t blinks = 1000 / (blink_period_ms * 2);
        
        for (uint32_t i = 0; i < blinks; i++) {
            gpio_put(LED_PIN, true);
            sleep_ms(blink_period_ms);
            gpio_put(LED_PIN, false);
            sleep_ms(blink_period_ms);
        }
    }
    
    // Final rapid blink to signal start
    for (int i = 0; i < 10; i++) {
        gpio_put(LED_PIN, true);
        sleep_ms(25);
        gpio_put(LED_PIN, false);
        sleep_ms(25);
    }
}

int main() {
    // CRITICAL: Boot delay prevents flash tool interference
    // This allows the flash process to complete cleanly
    sleep_ms(BOOT_DELAY_MS);
    
    // Initialize stdio for USB serial
    stdio_init_all();
    
    // LED setup for visual feedback
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);
    
    // Startup countdown - gives time to attach console
    startup_countdown();
    
    // Enable watchdog (8 second timeout)
    // Prevents system hangs during development
    watchdog_enable(8000, 1);
    
    printf("\n=================================\n");
    printf("PROJECT_NAME starting up...\n");
    printf("Board: PICO_BOARD_PLACEHOLDER\n");
    printf("Built with Pico SDK\n");
    printf("=================================\n\n");
    
    uint32_t loop_count = 0;
    
    while (true) {
        printf("Loop %lu: Hello, Pico!\n", ++loop_count);
        
        // Heartbeat LED
        gpio_put(LED_PIN, true);
        sleep_ms(250);
        gpio_put(LED_PIN, false);
        sleep_ms(250);
        
        // Feed watchdog to prevent reset
        watchdog_update();
    }
    
    return 0;
}