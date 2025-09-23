#include "shared_data.h"
#include <string.h>

// Global shared data instances
SharedData g_shared_data = {0};
SyncObjects g_sync;

void shared_data_init() {
    // Initialize shared data to safe defaults
    memset((void*)&g_shared_data, 0, sizeof(SharedData));
    g_shared_data.sample_rate_ms = 100;
    g_shared_data.led_brightness = 128;
    g_shared_data.led_enable = true;
    
    // Initialize synchronization objects
    mutex_init(&g_sync.data_mutex);
    sem_init(&g_sync.data_ready_sem, 0, 1);
    critical_section_init(&g_sync.critical_sec);
}

void set_sensor_data(float temp, uint16_t light, uint32_t count) {
    critical_section_enter_blocking(&g_sync.critical_sec);
    
    g_shared_data.temperature = temp;
    g_shared_data.light_level = light;
    g_shared_data.sample_count = count;
    
    critical_section_exit(&g_sync.critical_sec);
    
    // Signal that new data is available
    sem_release(&g_sync.data_ready_sem);
}

void get_sensor_data(float* temp, uint16_t* light, uint32_t* count) {
    critical_section_enter_blocking(&g_sync.critical_sec);
    
    if (temp) *temp = g_shared_data.temperature;
    if (light) *light = g_shared_data.light_level;
    if (count) *count = g_shared_data.sample_count;
    
    critical_section_exit(&g_sync.critical_sec);
}

void set_control_data(bool led_en, uint8_t brightness, uint32_t rate) {
    mutex_enter_blocking(&g_sync.data_mutex);
    
    g_shared_data.led_enable = led_en;
    g_shared_data.led_brightness = brightness;
    g_shared_data.sample_rate_ms = rate;
    
    mutex_exit(&g_sync.data_mutex);
}

void get_control_data(bool* led_en, uint8_t* brightness, uint32_t* rate) {
    mutex_enter_blocking(&g_sync.data_mutex);
    
    if (led_en) *led_en = g_shared_data.led_enable;
    if (brightness) *brightness = g_shared_data.led_brightness;
    if (rate) *rate = g_shared_data.sample_rate_ms;
    
    mutex_exit(&g_sync.data_mutex);
}

void update_statistics(uint32_t loop_time_us, float temperature) {
    static uint32_t temp_samples = 0;
    static float temp_sum = 0.0f;
    
    critical_section_enter_blocking(&g_sync.critical_sec);
    
    // Update maximum loop time
    if (loop_time_us > g_shared_data.max_loop_time_us) {
        g_shared_data.max_loop_time_us = loop_time_us;
    }
    
    // Update running average temperature
    temp_sum += temperature;
    temp_samples++;
    g_shared_data.avg_temperature = temp_sum / temp_samples;
    
    critical_section_exit(&g_sync.critical_sec);
}

void get_statistics(uint32_t* max_loop, float* avg_temp) {
    critical_section_enter_blocking(&g_sync.critical_sec);
    
    if (max_loop) *max_loop = g_shared_data.max_loop_time_us;
    if (avg_temp) *avg_temp = g_shared_data.avg_temperature;
    
    critical_section_exit(&g_sync.critical_sec);
}