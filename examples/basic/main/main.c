/* HOPE Badge BSP

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "bsp/bsp.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"

#include "vibramotor.h"

static const char *TAG = "badge main";

static TaskHandle_t led_blink_task_handle = NULL;
void led_rgb_blink_task(void *pvParameters);

static void btn_1_event_cb(void *arg, void *data)
{
    iot_button_print_event((button_handle_t)arg);

    if (led_blink_task_handle == NULL) {
        xTaskCreate(led_rgb_blink_task, "led_rgb_blink_task", 2048, NULL, 5, &led_blink_task_handle);
    } else {
        vTaskDelete(led_blink_task_handle);
        led_blink_task_handle = NULL;
    }

}

static void btn_2_event_cb(void *arg, void *data)
{
    iot_button_print_event((button_handle_t)arg);
}

static esp_err_t btn_register_callbacks(void)
{
    // Register callbacks for button events
    esp_err_t ret = iot_button_register_cb(bsp_get_button_handle(BSP_BUTTON_1_GPIO_INDEX), BUTTON_PRESS_DOWN, NULL, btn_1_event_cb, NULL);
    // ret |= iot_button_register_cb(bsp_get_button_handle(BSP_BUTTON_1_GPIO_INDEX), BUTTON_PRESS_UP, NULL, btn_1_event_cb, NULL);

    ret |= iot_button_register_cb(bsp_get_button_handle(BSP_BUTTON_2_GPIO_INDEX), BUTTON_DOUBLE_CLICK, NULL, btn_2_event_cb, NULL);

    button_event_args_t args = {
        .long_press.press_time = 5000,
    };

    ret |= iot_button_register_cb(bsp_get_button_handle(BSP_BUTTON_2_GPIO_INDEX), BUTTON_LONG_PRESS_START, &args, btn_2_event_cb, NULL);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register button callbacks: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

void led_rgb_blink_task(void *pvParameters)
{     
    bool led_on_off = false;

    led_strip_handle_t led_rgb = bsp_get_led_rgb_handle();
    if (led_rgb == NULL) {
        ESP_LOGE(TAG, "LED RGB handle is not initialized");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        vTaskDelete(NULL);
        return;
    }

    while(1) {
        if (led_on_off) {
            for (int i = 0; i < BSP_LED_RGB_PIXELS; i++) {
                ESP_ERROR_CHECK(led_strip_set_pixel(led_rgb, i, 5, 5, 5));
            }
            ESP_ERROR_CHECK(led_strip_refresh(led_rgb));
        } else {
            ESP_ERROR_CHECK(led_strip_clear(led_rgb));
        }
        led_on_off = !led_on_off;
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void led_rgb_ring_task(void *pvParameters)
{
    led_strip_handle_t led_rgb = bsp_get_led_rgb_handle();
    if (led_rgb == NULL) {
        ESP_LOGE(TAG, "LED RGB handle is not initialized");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        vTaskDelete(NULL);
        return;
    }

    int num_leds = BSP_LED_RGB_PIXELS; // assuming 16 or set explicitly to 16
    int current_led = 0;

    /* Steps to set the pixel
    
    1. Clear the strip
    2. Set the pixel at current_led to a color (e.g., purple)
    3. Refresh the strip to show the changes
    
    */

    while (1) {
        ESP_ERROR_CHECK(led_strip_clear(led_rgb));
        ESP_ERROR_CHECK(led_strip_set_pixel(led_rgb, current_led, 50, 0, 50));
        ESP_ERROR_CHECK(led_strip_refresh(led_rgb));
        current_led = (current_led + 1) % num_leds;
        vTaskDelay(40 / portTICK_PERIOD_MS); // Adjust delay for speed of ring
    }
}

void led_blink_task(void *pvParameters)
{     
    bool led_on_off = false;

    while(1) {
        if (led_on_off) {
            bsp_gpio_set_level(BSP_LED_IO, 0);
        } else {
            bsp_gpio_set_level(BSP_LED_IO, 1);
        }

        led_on_off = !led_on_off;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void led_battery_monitor_task(void *pvParameters)
{     
    
    float voltage = 0;
    float percentage = 0;

    while(1) {
        voltage = bsp_get_battery_voltage();
        percentage = bsp_get_battery_percentage();
        if (voltage < 0 || percentage < 0) {
            ESP_LOGE(TAG, "Failed to read battery voltage or percentage");
        } else {
            ESP_LOGI(TAG, "Battery Voltage: %.2f V, Percentage: %.2f%%", voltage, percentage);
        }
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting HOPE badge basic example");

    // Initialize I2C
    esp_err_t ret = bsp_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C: %s", esp_err_to_name(ret));
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

    // Start the LED RGB blink task
    // xTaskCreate(&led_rgb_blink_task, "led_rgb_blink_task", 2048, NULL, 6, NULL);
    // Start the LED RGB ring task
    // xTaskCreate(&led_rgb_ring_task, "led_rgb_ring_task", 2048, NULL, 8, NULL);
    // Start the LED blink task
    xTaskCreate(&led_blink_task, "led_blink_task", 2048, NULL, 7, NULL);
    // Start the LED battery monitor task
    xTaskCreate(&led_battery_monitor_task, "led_battery_monitor_task", 2048, NULL, 5, NULL);

    // Start the vibramotor run task for 500 ms on, 250 ms off, 6 cycles
    vibramotor_run(250, 100, 6);

    ESP_LOGI(TAG, "Hope Badge example initialized successfully");

}
