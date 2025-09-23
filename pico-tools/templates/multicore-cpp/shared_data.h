#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include "pico/stdlib.h"
#include "pico/sync.h"

// Shared data structure between cores
struct SharedData {
    // Sensor readings (updated by core1)
    volatile float temperature;
    volatile uint16_t light_level;
    volatile uint32_t sample_count;
    
    // Control signals (updated by core0)
    volatile bool led_enable;
    volatile uint8_t led_brightness;
    volatile uint32_t sample_rate_ms;
    
    // Status flags
    volatile bool core1_running;
    volatile uint32_t core0_heartbeat;
    volatile uint32_t core1_heartbeat;
    
    // Statistics
    volatile uint32_t max_loop_time_us;
    volatile float avg_temperature;
};

// Synchronization primitives
struct SyncObjects {
    mutex_t data_mutex;
    semaphore_t data_ready_sem;
    critical_section_t critical_sec;
};

// Global shared data
extern SharedData g_shared_data;
extern SyncObjects g_sync;

// Initialization
void shared_data_init();

// Thread-safe data access functions
void set_sensor_data(float temp, uint16_t light, uint32_t count);
void get_sensor_data(float* temp, uint16_t* light, uint32_t* count);
void set_control_data(bool led_en, uint8_t brightness, uint32_t rate);
void get_control_data(bool* led_en, uint8_t* brightness, uint32_t* rate);

// Statistics functions
void update_statistics(uint32_t loop_time_us, float temperature);
void get_statistics(uint32_t* max_loop, float* avg_temp);

#endif // SHARED_DATA_H