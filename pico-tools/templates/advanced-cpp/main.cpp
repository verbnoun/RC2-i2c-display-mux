#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/watchdog.h"
#include "sensor.h"
#include "display.h"

// Pin definitions
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
const uint BUTTON_PIN = 2;
const uint PWM_PIN = 15;
const uint ADC_PIN = 26;  // ADC0

// Global state
struct SystemState {
    float temperature;
    uint16_t light_level;
    bool button_pressed;
    uint32_t uptime_ms;
} system_state = {0};

void setup_hardware() {
    stdio_init_all();
    
    // LED setup
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // Button setup with pull-up
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    
    // PWM setup for LED brightness control
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    pwm_set_wrap(slice_num, 255);
    pwm_set_enabled(slice_num, true);
    
    // ADC setup
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);
    
    // Watchdog setup (8 second timeout)
    watchdog_enable(8000, 1);
    
    printf("PROJECT_NAME - Advanced Pico C++ Demo\n");
    printf("Board: PICO_BOARD_PLACEHOLDER\n");
    
    // Print unique ID
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    printf("Board ID: ");
    for (int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; i++) {
        printf("%02x", board_id.id[i]);
    }
    printf("\n");
}

void update_sensors() {
    // Read temperature (simulated from ADC noise + offset)
    adc_select_input(4);  // Temperature sensor
    uint16_t raw_temp = adc_read();
    system_state.temperature = 27.0f - (raw_temp * 3.3f / 4096.0f - 0.706f) / 0.001721f;
    
    // Read light level from ADC
    adc_select_input(0);
    system_state.light_level = adc_read();
    
    // Read button state
    system_state.button_pressed = !gpio_get(BUTTON_PIN);
    
    // Update uptime
    system_state.uptime_ms = to_ms_since_boot(get_absolute_time());
}

void update_outputs() {
    // Blink LED based on button state
    static uint32_t last_blink = 0;
    uint32_t blink_interval = system_state.button_pressed ? 100 : 500;
    
    if (system_state.uptime_ms - last_blink > blink_interval) {
        gpio_xor_mask(1u << LED_PIN);
        last_blink = system_state.uptime_ms;
    }
    
    // Set PWM brightness based on light level
    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    uint8_t brightness = (system_state.light_level >> 4) & 0xFF;
    pwm_set_gpio_level(PWM_PIN, brightness);
}

void print_status() {
    static uint32_t last_print = 0;
    
    if (system_state.uptime_ms - last_print > 2000) {
        printf("\n=== System Status ===\n");
        printf("Uptime: %.1f seconds\n", system_state.uptime_ms / 1000.0f);
        printf("Temperature: %.1f°C\n", system_state.temperature);
        printf("Light Level: %d/4095\n", system_state.light_level);
        printf("Button: %s\n", system_state.button_pressed ? "PRESSED" : "Released");
        printf("PWM Brightness: %d/255\n", (system_state.light_level >> 4) & 0xFF);
        
        // I2C sensor demo
        sensor_read_demo();
        
        // Display demo
        display_update_demo(system_state.temperature, system_state.light_level);
        
        last_print = system_state.uptime_ms;
    }
}

int main() {
    setup_hardware();
    
    // Initialize peripheral modules
    sensor_init();
    display_init();
    
    printf("System initialized. Starting main loop...\n");
    
    while (true) {
        // Update all sensor readings
        update_sensors();
        
        // Update outputs based on sensor data
        update_outputs();
        
        // Print periodic status
        print_status();
        
        // Feed the watchdog
        watchdog_update();
        
        // Small delay to prevent overwhelming the system
        sleep_ms(10);
    }
    
    return 0;
}