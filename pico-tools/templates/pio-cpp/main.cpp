#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812_driver.h"
#include "encoder_driver.h"
#include "uart_pio_driver.h"

// Generated PIO headers
#include "ws2812.pio.h"
#include "quadrature_encoder.pio.h"
#include "uart_tx.pio.h"

// Pin definitions
const uint WS2812_PIN = 2;
const uint ENCODER_CLK_PIN = 6;
const uint ENCODER_DATA_PIN = 7;
const uint UART_TX_PIN = 8;
const uint LED_PIN = PICO_DEFAULT_LED_PIN;

// Global state
struct SystemState {
    int32_t encoder_position;
    uint32_t led_color_index;
    uint32_t message_count;
    bool system_running;
} system_state = {0, 0, 0, true};

void setup_hardware() {
    stdio_init_all();
    
    // Setup built-in LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    printf("PROJECT_NAME - PIO Demonstration\n");
    printf("Board: PICO_BOARD_PLACEHOLDER\n");
    printf("PIO Features:\n");
    printf("  - WS2812 LED Strip Control\n");
    printf("  - Quadrature Encoder Reading\n");
    printf("  - Custom UART Transmission\n\n");
}

void demonstrate_pio_capabilities() {
    printf("=== PIO System Demonstration ===\n");
    
    // Initialize all PIO drivers
    ws2812_init(WS2812_PIN);
    encoder_init(ENCODER_CLK_PIN, ENCODER_DATA_PIN);
    uart_pio_init(UART_TX_PIN, 9600);
    
    printf("All PIO programs loaded and initialized\n");
    
    uint32_t last_update = 0;
    uint32_t last_encoder_pos = 0;
    uint32_t led_animation_step = 0;
    
    while (system_state.system_running) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        
        // Update encoder position
        system_state.encoder_position = encoder_get_position();
        
        // Check if encoder moved
        if (system_state.encoder_position != last_encoder_pos) {
            printf("Encoder: %d (moved %+d)\n", 
                   system_state.encoder_position, 
                   system_state.encoder_position - last_encoder_pos);
            
            // Send encoder update via PIO UART
            char uart_msg[64];
            snprintf(uart_msg, sizeof(uart_msg), "ENC:%d\r\n", system_state.encoder_position);
            uart_pio_puts(uart_msg);
            
            last_encoder_pos = system_state.encoder_position;
        }
        
        // Update LED strip animation every 100ms
        if (current_time - last_update > 100) {
            // Create rainbow effect based on encoder position and time
            uint32_t colors[8];
            for (int i = 0; i < 8; i++) {
                uint32_t hue = (led_animation_step * 4 + i * 32 + system_state.encoder_position * 2) & 0xFF;
                colors[i] = ws2812_hsv_to_rgb(hue, 255, 128); // Medium brightness
            }
            
            ws2812_put_pixels(colors, 8);
            
            // Blink built-in LED
            gpio_xor_mask(1u << LED_PIN);
            
            led_animation_step++;
            last_update = current_time;
        }
        
        // Send periodic status via PIO UART
        static uint32_t last_status = 0;
        if (current_time - last_status > 2000) {
            system_state.message_count++;
            
            char status_msg[128];
            snprintf(status_msg, sizeof(status_msg), 
                    "STATUS: Time=%ums, Enc=%d, Msgs=%u, LEDs=%u\r\n",
                    current_time, system_state.encoder_position, 
                    system_state.message_count, led_animation_step);
            
            uart_pio_puts(status_msg);
            
            printf("PIO Status: Messages=%u, Animation=%u, Encoder=%d\n",
                   system_state.message_count, led_animation_step, system_state.encoder_position);
            
            last_status = current_time;
        }
        
        // Check for user input to exit demo
        int c = getchar_timeout_us(0);
        if (c == 'q' || c == 'Q') {
            printf("Exiting PIO demonstration...\n");
            system_state.system_running = false;
        }
        
        sleep_ms(10);
    }
}

void run_pio_tests() {
    printf("\n=== PIO Hardware Tests ===\n");
    
    // Test 1: WS2812 LED Strip
    printf("Test 1: WS2812 LED Strip\n");
    printf("  Testing basic colors...\n");
    
    uint32_t test_colors[] = {
        0xFF0000,  // Red
        0x00FF00,  // Green
        0x0000FF,  // Blue
        0xFFFFFF,  // White
        0x000000   // Off
    };
    
    for (int color = 0; color < 5; color++) {
        uint32_t colors[8];
        for (int i = 0; i < 8; i++) {
            colors[i] = test_colors[color];
        }
        ws2812_put_pixels(colors, 8);
        printf("    Color %d applied\n", color);
        sleep_ms(500);
    }
    
    // Test 2: Encoder simulation (since real encoder might not be connected)
    printf("\nTest 2: Encoder Interface\n");
    printf("  Monitoring for encoder changes (5 seconds)...\n");
    
    int32_t start_pos = encoder_get_position();
    absolute_time_t test_end = make_timeout_time_ms(5000);
    
    while (!time_reached(test_end)) {
        int32_t current_pos = encoder_get_position();
        if (current_pos != start_pos) {
            printf("    Encoder moved: %d -> %d\n", start_pos, current_pos);
            start_pos = current_pos;
        }
        sleep_ms(50);
    }
    
    // Test 3: UART Transmission
    printf("\nTest 3: PIO UART Transmission\n");
    printf("  Sending test messages...\n");
    
    for (int i = 0; i < 5; i++) {
        char test_msg[64];
        snprintf(test_msg, sizeof(test_msg), "PIO_UART_TEST_%d\r\n", i);
        uart_pio_puts(test_msg);
        printf("    Sent: %s", test_msg);
        sleep_ms(200);
    }
    
    printf("\nAll PIO tests completed!\n");
}

int main() {
    setup_hardware();
    
    printf("PIO State Machines Available:\n");
    printf("  PIO0: %d state machines\n", NUM_PIO_STATE_MACHINES);
    printf("  PIO1: %d state machines\n", NUM_PIO_STATE_MACHINES);
    printf("  Total instruction memory: %d words per PIO\n", PIO_INSTRUCTION_COUNT);
    
    sleep_ms(1000);
    
    // Run hardware tests first
    run_pio_tests();
    
    printf("\nStarting interactive demonstration...\n");
    printf("Press 'q' to quit\n\n");
    
    // Run interactive demonstration
    demonstrate_pio_capabilities();
    
    // Cleanup - turn off all LEDs
    uint32_t off_colors[8] = {0};
    ws2812_put_pixels(off_colors, 8);
    gpio_put(LED_PIN, 0);
    
    printf("\nPIO demonstration complete!\n");
    printf("PIO Resources Summary:\n");
    printf("  WS2812 Driver: Uses precise timing for LED control\n");
    printf("  Encoder Reader: Real-time quadrature decoding\n");
    printf("  UART TX: Custom baud rate serial transmission\n");
    printf("  All running simultaneously with minimal CPU overhead\n");
    
    return 0;
}