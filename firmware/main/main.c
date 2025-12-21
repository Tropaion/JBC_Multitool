#include <stdio.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"

// Define ms delay
#define DELAY(ms)           vTaskDelay(pdMS_TO_TICKS(ms))
#define DEBOUNCE            DELAY(250)

// === RELAIS PINS ===
#define POWER_RELAIS        GPIO_NUM_21
#define SENSOR_RELAIS       GPIO_NUM_19
#define TOOL1_RELAIS        GPIO_NUM_18
#define TOOL2_RELAIS        GPIO_NUM_17
#define TOOL3_RELAIS        GPIO_NUM_16
#define TOOL4_RELAIS        GPIO_NUM_4

// === RELAIS STATUS VALUES ===
#define RELAIS_ENABLED      1
#define RELAIS_DISABLED     0

// === LED PINS ===
#define TOOL1_LED           GPIO_NUM_35
#define TOOL2_LED           GPIO_NUM_33
#define TOOL3_LED           GPIO_NUM_26
#define TOOL4_LED           GPIO_NUM_12

// === LED STATUS VALUES ===
#define LED_ENABLED         0
#define LED_DISABLED        1

// === SWITCH PINS ===
#define TOOL1_SWITCH        GPIO_NUM_34
#define TOOL2_SWITCH        GPIO_NUM_32
#define TOOL3_SWITCH        GPIO_NUM_25
#define TOOL4_SWITCH        GPIO_NUM_27

#define NUM_TOOLS           4
#define TOOL_SWITCH_PINS    {TOOL1_SWITCH, TOOL2_SWITCH, TOOL3_SWITCH, TOOL4_SWITCH}

// === SWITCH STATUS VALUES ===
#define SWITCH_ENABLED      0

// === ACTIVE TOOL STATUS VALUED ===
#define TOOLS_DISABLED      0
#define TOOL_1              1
#define TOOL_2              2
#define TOOL_3              3
#define TOOL_4              4

// === NVS KEY NAMES ===
#define NVS_NAMESPACE       "relay_storage"
#define ACTIVE_TOOL_KEY     "active_tool"

// Initializes input and output pins
void init_gpio()
{
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

// Initializes nvs
void init_nvs(nvs_handle_t *nvs_handle)
{
    // Initialize nvs
    esp_err_t err = nvs_flash_init();

    // Check for error
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // Ersase  nvs
        ESP_ERROR_CHECK(nvs_flash_erase());

        // Recreate nvs
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open nvs handle
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, nvs_handle);

    // Check for error
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to open NVS handle!");
        return;
    }
}

// Sets everything to disabled
void disable_all()
{
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

// General set_tool function
void set_tool(uint8_t *active_tool, uint8_t tool_number)
{
    // Debounce
    if (*active_tool == tool_number) {
        DELAY(200);
        return;
    }

    // Disable all tools and relays
    disable_all();

    // Set the tool's relay and LED based on the tool number
    switch (tool_number)
    {
        case TOOLS_DISABLED:
            // Do nothing
            *active_tool = TOOLS_DISABLED;
            return;
        case TOOL_1:
            gpio_set_level(SENSOR_RELAIS, RELAIS_ENABLED);
            gpio_set_level(TOOL1_RELAIS, RELAIS_ENABLED);
            gpio_set_level(TOOL1_LED, LED_ENABLED);
            break;
        case TOOL_2:
            gpio_set_level(TOOL2_RELAIS, RELAIS_ENABLED);
            gpio_set_level(TOOL2_LED, LED_ENABLED);
            break;
        case TOOL_3:
            gpio_set_level(TOOL3_RELAIS, RELAIS_ENABLED);
            gpio_set_level(TOOL3_LED, LED_ENABLED);
            break;
        case TOOL_4:
            gpio_set_level(TOOL4_RELAIS, RELAIS_ENABLED);
            gpio_set_level(TOOL4_LED, LED_ENABLED);
            break;
        default:
            ESP_LOGE("set_tool", "Invalid tool number: %d", tool_number);
            return;
    }

    // Enable the power relay
    gpio_set_level(POWER_RELAIS, RELAIS_ENABLED);

    // Save the active tool to NVS
    *active_tool = tool_number;
}

// Loads last tool state
void nvs_load_tool_state(nvs_handle_t nvs_handle, uint8_t *active_tool)
{
    uint8_t prev_tool_state = TOOLS_DISABLED;

    // Get relay state from last shutdown to restore
    esp_err_t err = nvs_get_u8(nvs_handle, ACTIVE_TOOL_KEY, &prev_tool_state);

    if (err == ESP_OK)
    {
        ESP_LOGI("NVS", "Restored active tool: %d", active_tool);
    } else
    {
        ESP_LOGE("NVS", "Failed to load active tool!");
    }

    // Restore relay and LED state based on the loaded tool
    set_tool(active_tool, prev_tool_state);
}

// Saves tool state
void nvs_save_tool_state(nvs_handle_t nvs_handle, uint8_t tool_state)
{
    esp_err_t err = nvs_set_u8(nvs_handle, ACTIVE_TOOL_KEY, tool_state);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "Failed to save active tool!");
    }

    // Write changes to storage
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE("NVS", "Failed to commit NVS!");
    }
}

// Main application
void app_main(void)
{
    // Create required variables
    gpio_num_t tool_switch_pins[NUM_TOOLS] = TOOL_SWITCH_PINS;
    nvs_handle_t nvs_handle = 0;
    uint8_t active_tool = TOOLS_DISABLED;

    // Initialize GPIOs
    init_gpio();

    // Disable periphery
    disable_all();

    // Initialize nvs and restore last tool state
    init_nvs(&nvs_handle);
    nvs_load_tool_state(nvs_handle, &active_tool);

    while(1)
    {
        // Loop through every tool input button
        for (int i = 0; i < NUM_TOOLS; i++)
        {
            // Check if the current tool switch was pressed
            if (gpio_get_level(tool_switch_pins[i]) == SWITCH_ENABLED)
            {
                // Enable the tool based on the loop index (i + 1 for tool numbers)
                set_tool(&active_tool, i + 1);

                // Save tool state to nvs
                nvs_save_tool_state(nvs_handle, active_tool);

                // Wait debounce time
                DEBOUNCE;

                // Wait for the button to be released
                while (gpio_get_level(tool_switch_pins[i]) == SWITCH_ENABLED)
                {
                    // Wait debounce time
                    DEBOUNCE;
                }
            }
        }

        // Let IDLE0 task reset watchdog
        vTaskDelay(1);
    }
}
