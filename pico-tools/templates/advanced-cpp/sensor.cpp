#include "sensor.h"
#include <stdio.h>
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

// I2C configuration
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5
#define I2C_FREQ 100000

// SPI configuration
#define SPI_PORT spi0
#define SPI_MISO 16
#define SPI_MOSI 19
#define SPI_SCK 18
#define SPI_CS 17

void sensor_init() {
    // Initialize I2C
    i2c_init(I2C_PORT, I2C_FREQ);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    printf("I2C sensor interface initialized\n");
}

void sensor_read_demo() {
    // Demo I2C device scan
    static bool scan_done = false;
    if (!scan_done) {
        printf("Scanning I2C bus...\n");
        
        for (int addr = 0x08; addr < 0x78; addr++) {
            uint8_t rxdata;
            int ret = i2c_read_blocking(I2C_PORT, addr, &rxdata, 1, false);
            if (ret >= 0) {
                printf("Found I2C device at 0x%02X\n", addr);
            }
        }
        scan_done = true;
    }
}

float sensor_read_temperature() {
    // Example for a temperature sensor (like BME280 or similar)
    // This is a placeholder - replace with actual sensor communication
    
    uint8_t cmd = 0xF3; // Example command
    uint8_t data[2];
    
    // Write command to sensor
    i2c_write_blocking(I2C_PORT, 0x40, &cmd, 1, true);
    
    // Read response
    int result = i2c_read_blocking(I2C_PORT, 0x40, data, 2, false);
    
    if (result == 2) {
        uint16_t raw = (data[0] << 8) | data[1];
        // Convert raw value to temperature (example formula)
        return (raw * 175.72f / 65536.0f) - 46.85f;
    }
    
    return -999.0f; // Error value
}

uint16_t sensor_read_humidity() {
    // Example humidity reading
    // Replace with actual sensor implementation
    uint8_t cmd = 0xF5;
    uint8_t data[2];
    
    i2c_write_blocking(I2C_PORT, 0x40, &cmd, 1, true);
    int result = i2c_read_blocking(I2C_PORT, 0x40, data, 2, false);
    
    if (result == 2) {
        uint16_t raw = (data[0] << 8) | data[1];
        return (uint16_t)(raw * 125.0f / 65536.0f - 6.0f);
    }
    
    return 0;
}

void spi_sensor_init() {
    // Initialize SPI
    spi_init(SPI_PORT, 1000000); // 1MHz
    
    gpio_set_function(SPI_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SCK, GPIO_FUNC_SPI);
    
    // CS pin as regular GPIO
    gpio_init(SPI_CS);
    gpio_set_dir(SPI_CS, GPIO_OUT);
    gpio_put(SPI_CS, 1); // CS high (inactive)
    
    printf("SPI sensor interface initialized\n");
}

uint32_t spi_sensor_read() {
    uint8_t tx_data[4] = {0x00, 0x00, 0x00, 0x00}; // Example command
    uint8_t rx_data[4];
    
    // Select device
    gpio_put(SPI_CS, 0);
    
    // Transfer data
    spi_write_read_blocking(SPI_PORT, tx_data, rx_data, 4);
    
    // Deselect device
    gpio_put(SPI_CS, 1);
    
    // Combine bytes into 32-bit value
    return (rx_data[0] << 24) | (rx_data[1] << 16) | (rx_data[2] << 8) | rx_data[3];
}