#include <stdio.h>
#include "driver\gpio.h"
#include "freeRTOS\freeRTOS.h"
#include "freeRTOS\task.h"

// Define ms delay
#define DELAY(ms)           vTaskDelay(pdMS_TO_TICKS(ms))

// === RELAIS PINS ===
#define POWER_RELAIS        GPIO_NUM_19
#define SENSOR_RELAIS       GPIO_NUM_18
#define TOOL1_RELAIS        GPIO_NUM_5
#define TOOL2_RELAIS        GPIO_NUM_17
#define TOOL3_RELAIS        GPIO_NUM_16
#define TOOL4_RELAIS        GPIO_NUM_4

#define RELAIS_ENABLED      1
#define RELAIS_DISABLED     0

// === LED PINS ===
#define TOOL1_LED           GPIO_NUM_32
#define TOOL2_LED           GPIO_NUM_25
#define TOOL3_LED           GPIO_NUM_27
// To use this pin, set fuse: espefuse.py --port COM5 set_flash_voltage 3.3V
#define TOOL4_LED           GPIO_NUM_12

#define LED_ENABLED         0
#define LED_DISABLED        1

// === SWITCH PINS ===
#define TOOL1_SWITCH        GPIO_NUM_35
#define TOOL2_SWITCH        GPIO_NUM_33
#define TOOL3_SWITCH        GPIO_NUM_26
#define TOOL4_SWITCH        GPIO_NUM_14

#define SWITCH_DISABLED     0

// Variable which saves active tool
int active_tool = 0;

// Configures input and output pins
void configure_gpio() {
    // Configure output pins
    gpio_config_t output_conf = {};
    output_conf.mode = GPIO_MODE_OUTPUT;
    output_conf.pin_bit_mask = (1ULL << POWER_RELAIS) |
                           (1ULL << SENSOR_RELAIS) |
                           (1ULL << TOOL1_RELAIS) |
                           (1ULL << TOOL2_RELAIS) |
                           (1ULL << TOOL3_RELAIS) |
                           (1ULL << TOOL4_RELAIS) |
                           (1ULL << TOOL1_LED) |
                           (1ULL << TOOL2_LED) |
                           (1ULL << TOOL3_LED) |
                           (1ULL << TOOL4_LED);
    output_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    output_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    output_conf.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&output_conf));

    // Configure input pins
    gpio_config_t input_conf = {};
    input_conf.mode = GPIO_MODE_INPUT;
    input_conf.pin_bit_mask = (1ULL << TOOL1_SWITCH) | 
                           (1ULL << TOOL2_SWITCH) | 
                           (1ULL << TOOL3_SWITCH) | 
                           (1ULL << TOOL4_SWITCH);
    input_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    input_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    input_conf.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&input_conf));
}

// Sets everything to disabled
void disable_all() {
    // Disable all relais
    gpio_set_level(POWER_RELAIS, RELAIS_DISABLED);
    gpio_set_level(SENSOR_RELAIS, RELAIS_DISABLED);
    gpio_set_level(TOOL1_RELAIS, RELAIS_DISABLED);
    gpio_set_level(TOOL2_RELAIS, RELAIS_DISABLED);
    gpio_set_level(TOOL3_RELAIS, RELAIS_DISABLED);
    gpio_set_level(TOOL4_RELAIS, RELAIS_DISABLED);

    // Disable all leds
    gpio_set_level(TOOL1_LED, LED_DISABLED);
    gpio_set_level(TOOL2_LED, LED_DISABLED);
    gpio_set_level(TOOL3_LED, LED_DISABLED);
    gpio_set_level(TOOL4_LED, LED_DISABLED);
}

// Enables tool 1 (t210)
void set_tool_1() {
    // Check if tool is aready active
    if (active_tool == 1) {
        // Debounce
        DELAY(200);
        return;
    }

    // Disable everything
    disable_all();

    // Enable required relais
    // T210, small soldering pen
    gpio_set_level(SENSOR_RELAIS, RELAIS_ENABLED);
    gpio_set_level(TOOL1_RELAIS, RELAIS_ENABLED);

    // Wait for JBC
    DELAY(300);

    // Enable power
    gpio_set_level(POWER_RELAIS, RELAIS_ENABLED);
  
    // Enable led
    gpio_set_level(TOOL1_LED, LED_ENABLED);
  
    // Set active tool
    active_tool = 1;
}

// Enables tool 2
void set_tool_2() {
    // Check if tool is aready active
    if (active_tool == 2) {
        // Debounce
        DELAY(200);
        return;
    }
  
    // Disable everything
    disable_all();
  
    // Enable required relais
    gpio_set_level(TOOL2_RELAIS, RELAIS_ENABLED);
  
    // Wait for JBC
    DELAY(300);
  
    // Enable power
    gpio_set_level(POWER_RELAIS, RELAIS_ENABLED);
  
    // Enable led
    gpio_set_level(TOOL2_LED, LED_ENABLED);
  
    // Set active tool
    active_tool = 2;
}

// Enables tool 3
void set_tool_3() {
    // Check if tool is aready active
    if (active_tool == 2) {
        // Debounce
        DELAY(200);
        return;
    }
  
    // Disable everything
    disable_all();
  
    // Enable required relais
    gpio_set_level(TOOL3_RELAIS, RELAIS_ENABLED);
  
    // Wait for JBC
    DELAY(300);
  
    // Enable power
    gpio_set_level(POWER_RELAIS, RELAIS_ENABLED);
  
    // Enable led
    gpio_set_level(TOOL3_LED, LED_ENABLED);
  
    // Set active tool
    active_tool = 3;
}

// Enables tool 4
void set_tool_4() {
    // Check if tool is aready active
    if (active_tool == 4) {
        // Debounce
        DELAY(200);
        return;
    }
  
    // Disable everything
    disable_all();
  
    // Enable required relais
    gpio_set_level(TOOL4_RELAIS, RELAIS_ENABLED);
  
    // Wait for JBC
    DELAY(300);
  
    // Enable power
    gpio_set_level(POWER_RELAIS, RELAIS_ENABLED);
  
    // Enable led
    gpio_set_level(TOOL4_LED, LED_ENABLED);
  
    // Set active tool
    active_tool = 4;
}

void app_main(void)
{
    // Configure digital I/O PINs
    configure_gpio();

    // Disable periphery
    disable_all();

    while(1)
    {
        // Check if button was pressed
        // if (gpio_get_level(TOOL1_SWITCH) == SWITCH_DISABLED) {
        //     set_tool_1();
        // }
        // Check if button was pressed
        if (gpio_get_level(TOOL2_SWITCH) == SWITCH_DISABLED) {
            set_tool_2();
        }

        // Check if button was pressed
        if (gpio_get_level(TOOL3_SWITCH) == SWITCH_DISABLED) {
            set_tool_3();
        }

        // Check if button was pressed
        if (gpio_get_level(TOOL4_SWITCH) == SWITCH_DISABLED) {
            set_tool_4();
        }

        // Let IDLE0 task reset watchdog
        vTaskDelay(1);
    }
}