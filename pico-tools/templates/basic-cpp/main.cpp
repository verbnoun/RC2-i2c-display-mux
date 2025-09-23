#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

int main() {
    stdio_init_all();
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    printf("PROJECT_NAME starting up...\n");
    printf("Board: PICO_BOARD_PLACEHOLDER\n");
    printf("Built with Pico SDK\n");
    
    while (true) {
        printf("Hello, Pico!\n");
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
    
    return 0;
}