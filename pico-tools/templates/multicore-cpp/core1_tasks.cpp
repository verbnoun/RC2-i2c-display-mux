#include "core1_tasks.h"
#include "shared_data.h"
#include <stdio.h>
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/time.h"

// Core1 pin definitions
const uint TEMP_ADC_PIN = 26;  // ADC0
const uint LIGHT_ADC_PIN = 27; // ADC1

void core1_main() {
    printf("Core1: Starting up...\n");
    
    // Initialize ADC for sensor reading
    adc_init();
    adc_gpio_init(TEMP_ADC_PIN);
    adc_gpio_init(LIGHT_ADC_PIN);
    
    // Mark core1 as running
    g_shared_data.core1_running = true;
    
    printf("Core1: Initialized and ready\n");
    
    uint32_t loop_count = 0;
    absolute_time_t loop_start;
    
    while (core1_should_continue()) {
        loop_start = get_absolute_time();
        
        // Execute core1 tasks
        core1_sensor_task();
        core1_processing_task();
        core1_communication_task();
        
        // Update heartbeat
        core1_heartbeat_update();
        
        // Calculate loop timing for performance monitoring
        uint32_t loop_time_us = absolute_time_diff_us(loop_start, get_absolute_time());
        
        // Get current sensor data for statistics
        float temp;
        get_sensor_data(&temp, nullptr, nullptr);
        update_statistics(loop_time_us, temp);
        
        loop_count++;
        
        // Get current sample rate from core0
        uint32_t sample_rate_ms;
        get_control_data(nullptr, nullptr, &sample_rate_ms);
        
        // Sleep for the configured sample rate
        sleep_ms(sample_rate_ms);
    }
    
    printf("Core1: Shutting down after %u loops\n", loop_count);
    g_shared_data.core1_running = false;
}

void core1_sensor_task() {
    static uint32_t sample_count = 0;
    
    // Read temperature from ADC (using internal temperature sensor)
    adc_select_input(4); // Internal temperature sensor
    uint16_t raw_temp = adc_read();
    float temperature = 27.0f - (raw_temp * 3.3f / 4096.0f - 0.706f) / 0.001721f;
    
    // Read light level from external ADC
    adc_select_input(1); // LIGHT_ADC_PIN (ADC1)
    uint16_t light_level = adc_read();
    
    // Update shared data with new sensor readings
    sample_count++;
    set_sensor_data(temperature, light_level, sample_count);
}

void core1_processing_task() {
    // Get current sensor data for processing
    float temperature;
    uint16_t light_level;
    uint32_t sample_count;
    get_sensor_data(&temperature, &light_level, &sample_count);
    
    // Example processing: Adaptive brightness based on light and temperature
    uint8_t calculated_brightness = 255;
    
    // Reduce brightness in bright light
    if (light_level > 3000) {
        calculated_brightness = 64;
    } else if (light_level > 2000) {
        calculated_brightness = 128;
    } else if (light_level > 1000) {
        calculated_brightness = 192;
    }
    
    // Reduce brightness if temperature is high (thermal protection)
    if (temperature > 30.0f) {
        calculated_brightness = calculated_brightness / 2;
    }
    
    // Update control data with processed values
    bool led_enable = true;
    uint32_t current_rate_ms;
    get_control_data(&led_enable, nullptr, &current_rate_ms);
    
    set_control_data(led_enable, calculated_brightness, current_rate_ms);
}

void core1_communication_task() {
    // Example: Inter-core communication demonstration
    static uint32_t last_report = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Report status every 5 seconds
    if (current_time - last_report > 5000) {
        float temperature;
        uint16_t light_level;
        uint32_t sample_count;
        get_sensor_data(&temperature, &light_level, &sample_count);
        
        printf("Core1 Report: Temp=%.1f°C, Light=%d, Samples=%u\n", 
               temperature, light_level, sample_count);
        
        last_report = current_time;
    }
    
    // Signal that data is ready for core0 to process
    sem_release(&g_sync.data_ready_sem);
}

void core1_heartbeat_update() {
    // Update heartbeat counter for core0 to monitor
    g_shared_data.core1_heartbeat++;
}

bool core1_should_continue() {
    // Core1 continues running while core0 is active
    // This could be controlled by a shutdown flag from core0
    return g_shared_data.core1_running;
}