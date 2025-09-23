#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "shared_data.h"
#include "core1_tasks.h"

// Core0 pin definitions
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
const uint BUTTON_PIN = 2;
const uint PWM_PIN = 15;

// Core0 state
struct Core0State {
    bool led_state;
    bool button_pressed;
    bool button_last_state;
    uint32_t button_press_count;
    uint32_t last_button_time;
    uint32_t last_status_time;
} core0_state = {0};

void setup_core0_hardware() {
    stdio_init_all();
    
    // LED setup
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // Button setup with pull-up
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    
    // PWM setup
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    pwm_set_wrap(slice_num, 255);
    pwm_set_enabled(slice_num, true);
    
    printf("PROJECT_NAME - Multicore Pico C++ Demo\n");
    printf("Board: PICO_BOARD_PLACEHOLDER\n");
    printf("Core0: Hardware initialized\n");
}

void handle_button_input() {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    bool button_current = !gpio_get(BUTTON_PIN);
    
    // Debounce button (50ms)
    if (button_current != core0_state.button_last_state) {
        if (current_time - core0_state.last_button_time > 50) {
            core0_state.button_pressed = button_current;
            
            if (button_current && !core0_state.button_last_state) {
                // Button press detected
                core0_state.button_press_count++;
                printf("Core0: Button pressed (count: %u)\n", core0_state.button_press_count);
                
                // Toggle LED enable state
                bool led_enable, led_brightness;
                uint32_t sample_rate;
                get_control_data(&led_enable, &led_brightness, &sample_rate);
                
                // Cycle through sample rates: 50ms, 100ms, 200ms, 500ms
                uint32_t new_rate = sample_rate;
                switch (sample_rate) {
                    case 50: new_rate = 100; break;
                    case 100: new_rate = 200; break;
                    case 200: new_rate = 500; break;
                    case 500: new_rate = 50; break;
                    default: new_rate = 100; break;
                }
                
                set_control_data(!led_enable, led_brightness, new_rate);
                printf("Core0: LED %s, Sample rate: %ums\n", 
                       !led_enable ? "ON" : "OFF", new_rate);
            }
            
            core0_state.last_button_time = current_time;
        }
        core0_state.button_last_state = button_current;
    }
}

void update_outputs() {
    // Get control data from shared memory
    bool led_enable;
    uint8_t led_brightness;
    get_control_data(&led_enable, &led_brightness, nullptr);
    
    // Update LED state
    if (led_enable) {
        static uint32_t last_blink = 0;
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        
        // Blink rate based on button state
        uint32_t blink_interval = core0_state.button_pressed ? 100 : 500;
        
        if (current_time - last_blink > blink_interval) {
            core0_state.led_state = !core0_state.led_state;
            gpio_put(LED_PIN, core0_state.led_state);
            last_blink = current_time;
        }
    } else {
        gpio_put(LED_PIN, 0);
        core0_state.led_state = false;
    }
    
    // Update PWM brightness
    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    pwm_set_gpio_level(PWM_PIN, led_brightness);
}

void print_system_status() {
    static uint32_t last_print = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if (current_time - last_print > 3000) { // Every 3 seconds
        // Get sensor data
        float temperature;
        uint16_t light_level;
        uint32_t sample_count;
        get_sensor_data(&temperature, &light_level, &sample_count);
        
        // Get control data
        bool led_enable;
        uint8_t led_brightness;
        uint32_t sample_rate;
        get_control_data(&led_enable, &led_brightness, &sample_rate);
        
        // Get statistics
        uint32_t max_loop_time;
        float avg_temperature;
        get_statistics(&max_loop_time, &avg_temperature);
        
        printf("\n=== Multicore System Status ===\n");
        printf("Core0 Uptime: %.1f seconds\n", current_time / 1000.0f);
        printf("Core1 Running: %s\n", g_shared_data.core1_running ? "YES" : "NO");
        printf("Core0 Heartbeat: %u\n", g_shared_data.core0_heartbeat);
        printf("Core1 Heartbeat: %u\n", g_shared_data.core1_heartbeat);
        printf("\nSensor Data:\n");
        printf("  Temperature: %.1f°C (avg: %.1f°C)\n", temperature, avg_temperature);
        printf("  Light Level: %d/4095\n", light_level);
        printf("  Sample Count: %u\n", sample_count);
        printf("  Sample Rate: %ums\n", sample_rate);
        printf("\nControl State:\n");
        printf("  LED Enable: %s\n", led_enable ? "ON" : "OFF");
        printf("  LED Brightness: %d/255\n", led_brightness);
        printf("  Button Presses: %u\n", core0_state.button_press_count);
        printf("\nPerformance:\n");
        printf("  Max Loop Time: %uus\n", max_loop_time);
        
        last_print = current_time;
        core0_state.last_status_time = current_time;
    }
}

void monitor_core1_health() {
    static uint32_t last_core1_heartbeat = 0;
    static uint32_t last_check = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Check core1 health every second
    if (current_time - last_check > 1000) {
        if (g_shared_data.core1_heartbeat == last_core1_heartbeat) {
            printf("Core0: WARNING - Core1 appears stalled!\n");
        }
        last_core1_heartbeat = g_shared_data.core1_heartbeat;
        last_check = current_time;
    }
}

int main() {
    setup_core0_hardware();
    
    // Initialize shared data structures
    shared_data_init();
    
    printf("Core0: Launching Core1...\n");
    
    // Start core1
    multicore_launch_core1(core1_main);
    
    // Wait for core1 to initialize
    while (!g_shared_data.core1_running) {
        sleep_ms(10);
    }
    
    printf("Core0: Both cores running, starting main loop\n");
    
    while (true) {
        // Update core0 heartbeat
        g_shared_data.core0_heartbeat++;
        
        // Handle user input
        handle_button_input();
        
        // Update outputs based on shared data
        update_outputs();
        
        // Print periodic status
        print_system_status();
        
        // Monitor core1 health
        monitor_core1_health();
        
        // Wait for new data from core1 (with timeout)
        bool got_data = sem_acquire_timeout_ms(&g_sync.data_ready_sem, 100);
        if (!got_data) {
            printf("Core0: Timeout waiting for Core1 data\n");
        }
        
        // Small delay to prevent overwhelming the system
        sleep_ms(10);
    }
    
    return 0;
}