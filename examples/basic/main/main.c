/* HOPE Badge BSP

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "iot_button.h"
#include "led_strip.h"

#include "bsp/bsp.h"
#include "vibramotor.h"

static const char *TAG = "badge main";

static TaskHandle_t led_rgb_task_handle = NULL;
void led_rgb_blink_task(void *pvParameters);
void led_rgb_ring_task(void *pvParameters);

static void btn_1_event_cb(void *arg, void *data)
{
    iot_button_print_event((button_handle_t)arg);

    if (led_rgb_task_handle != NULL) {
        vTaskDelete(led_rgb_task_handle);
        led_rgb_task_handle = NULL;
        // Clear the strip when stopping
        led_strip_handle_t led_rgb = bsp_get_led_rgb_handle();
        if (led_rgb) {
            led_strip_clear(led_rgb);
        }
    } else {
        xTaskCreate(led_rgb_blink_task, "led_rgb_blink_task", 2048, NULL, 5, &led_rgb_task_handle);
    }
}

static void btn_2_event_cb(void *arg, void *data)
{
    iot_button_print_event((button_handle_t)arg);
}

static esp_err_t btn_register_callbacks(void)
{
    esp_err_t ret = ESP_OK;
    esp_err_t err;

    // Register callbacks for button events
    err = iot_button_register_cb(bsp_get_button_handle(BSP_BUTTON_1_GPIO_INDEX), BUTTON_PRESS_DOWN, NULL, btn_1_event_cb, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register button 1 callback: %s", esp_err_to_name(err));
        if (ret == ESP_OK) ret = err;
    }

    err = iot_button_register_cb(bsp_get_button_handle(BSP_BUTTON_2_GPIO_INDEX), BUTTON_DOUBLE_CLICK, NULL, btn_2_event_cb, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register button 2 double-click callback: %s", esp_err_to_name(err));
        if (ret == ESP_OK) ret = err;
    }

    button_event_args_t args = {
        .long_press.press_time = 5000,
    };

    err = iot_button_register_cb(bsp_get_button_handle(BSP_BUTTON_2_GPIO_INDEX), BUTTON_LONG_PRESS_START, &args, btn_2_event_cb, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register button 2 long-press callback: %s", esp_err_to_name(err));
        if (ret == ESP_OK) ret = err;
    }

    return ret;
}

void led_rgb_blink_task(void *pvParameters)
{
    bool led_on_off = false;

    led_strip_handle_t led_rgb = bsp_get_led_rgb_handle();
    if (led_rgb == NULL) {
        ESP_LOGE(TAG, "LED RGB handle is not initialized");
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        esp_err_t ret;
        if (led_on_off) {
            for (int i = 0; i < BSP_LED_RGB_PIXELS; i++) {
                ret = led_strip_set_pixel(led_rgb, i, 5, 5, 5);
                if (ret != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to set pixel %d: %s", i, esp_err_to_name(ret));
                    break;
                }
            }
            led_strip_refresh(led_rgb);
        } else {
            led_strip_clear(led_rgb);
        }
        led_on_off = !led_on_off;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void led_rgb_ring_task(void *pvParameters)
{
    led_strip_handle_t led_rgb = bsp_get_led_rgb_handle();
    if (led_rgb == NULL) {
        ESP_LOGE(TAG, "LED RGB handle is not initialized");
        vTaskDelete(NULL);
        return;
    }

    int num_leds = BSP_LED_RGB_PIXELS;
    int current_led = 0;

    /* Steps to set the pixel
    
    1. Clear the strip
    2. Set the pixel at current_led to a color (e.g., purple)
    3. Refresh the strip to show the changes
    
    */

    while (1) {
        led_strip_clear(led_rgb);
        led_strip_set_pixel(led_rgb, current_led, 50, 0, 50);
        led_strip_refresh(led_rgb);
        current_led = (current_led + 1) % num_leds;
        vTaskDelay(pdMS_TO_TICKS(40)); // Adjust delay for speed of ring
    }
}

void led_blink_task(void *pvParameters)
{
    bool led_on_off = false;

    while (1) {
        if (led_on_off) {
            bsp_gpio_set_level(BSP_LED_IO, 0);
        } else {
            bsp_gpio_set_level(BSP_LED_IO, 1);
        }

        led_on_off = !led_on_off;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void led_battery_monitor_task(void *pvParameters)
{
    float voltage = 0;
    float percentage = 0;

    while (1) {
        voltage = bsp_get_battery_voltage();
        percentage = bsp_get_battery_percentage();
        if (voltage < 0 || percentage < 0) {
            ESP_LOGE(TAG, "Failed to read battery voltage or percentage");
        } else {
            ESP_LOGI(TAG, "Battery Voltage: %.2f V, Percentage: %.2f%%", voltage, percentage);
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting HOPE badge basic example");

    // Initialize BSP (I2C, buttons, LEDs, fuel gauge, etc.)
    esp_err_t ret = bsp_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize BSP: %s", esp_err_to_name(ret));
        return;
    }

    // Register callbacks for button events
    ret = btn_register_callbacks();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register button callbacks: %s", esp_err_to_name(ret));
        return;
    }

    // Initialize the vibramotor
    ret = vibramotor_init(BSP_VIBRAMOTOR_IO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize vibramotor: %s", esp_err_to_name(ret));
        return;
    }

    // Start the LED RGB ring task (button 1 toggles to blink mode)
    xTaskCreate(led_rgb_ring_task, "led_rgb_ring_task", 2048, NULL, 8, &led_rgb_task_handle);

    // Start the LED blink task
    xTaskCreate(led_blink_task, "led_blink_task", 2048, NULL, 7, NULL);
    // Start the battery monitor task (needs extra stack for float formatting)
    xTaskCreate(led_battery_monitor_task, "battery_monitor", 3072, NULL, 5, NULL);

    // Start the vibramotor run task for 250 ms on, 100 ms off, for 6 cycles
    vibramotor_run(250, 100, 6);

    ESP_LOGI(TAG, "Hope Badge example initialized successfully");

}
