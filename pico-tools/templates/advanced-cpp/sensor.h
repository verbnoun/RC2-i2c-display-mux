#ifndef SENSOR_H
#define SENSOR_H

#include "pico/stdlib.h"

// I2C sensor interface
void sensor_init();
void sensor_read_demo();
float sensor_read_temperature();
uint16_t sensor_read_humidity();

// SPI sensor interface  
void spi_sensor_init();
uint32_t spi_sensor_read();

#endif // SENSOR_H