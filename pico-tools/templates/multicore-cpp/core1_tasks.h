#ifndef CORE1_TASKS_H
#define CORE1_TASKS_H

#include "pico/stdlib.h"

// Core1 main function
void core1_main();

// Core1 task functions
void core1_sensor_task();
void core1_processing_task();
void core1_communication_task();

// Core1 utility functions
void core1_heartbeat_update();
bool core1_should_continue();

#endif // CORE1_TASKS_H