/* HOPE Badge BSP

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "vibrator.h"
#include <stdbool.h>
#include <stdint.h>

static const char *TAG = "Vibrator";
static int8_t vibrator_gpio_num = -1;

void vibrator_run_task(void *pvParameters)
{
    vibrator_params_t *params = (vibrator_params_t *) pvParameters;

    if (params == NULL) {
        ESP_LOGE(TAG, "Invalid parameters for vibrator task");
        vTaskDelete(NULL);
        return;
    }
    
    uint32_t on_time = params->time_on_ms;
    uint32_t off_time = params->time_off_ms;
    uint32_t repeat_count = params->repeat_count;

    for (uint32_t i = 0; i < repeat_count; i++) {
        gpio_set_level(vibrator_gpio_num, 1);
        vTaskDelay(on_time / portTICK_PERIOD_MS);
        gpio_set_level(vibrator_gpio_num, 0);
        vTaskDelay(off_time / portTICK_PERIOD_MS);
    }
    // Optionally delete task after completion
    vTaskDelete(NULL);
}


esp_err_t vibrator_stop(void)
{
    esp_err_t ret = ESP_OK;
    return ret;
}

esp_err_t vibrator_run(uint16_t time_on_ms, uint16_t time_off_ms, uint16_t cycles)
{
    esp_err_t ret = ESP_OK;

    vibrator_params_t params = {
        .time_on_ms = time_on_ms,
        .time_off_ms = time_off_ms,
        .repeat_count = cycles,
    };

    // Create a task to run the vibrator
    if (vibrator_gpio_num == -1) {
        ESP_LOGE(TAG, "Vibrator GPIO not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ret = xTaskCreate(vibrator_run_task, "vibrator_task", 2048, &params, 5, NULL);
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create vibrator task: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    return ret;
}

/**
 * @brief Initialize the vibrator by configuring the GPIO pin.
 *
 * @param gpio_num The GPIO number to which the vibrator is connected.
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if the GPIO number is invalid
 *      - Other error codes on failure to configure the GPIO
 */

esp_err_t vibrator_init(uint8_t gpio_num)
{
    ESP_LOGI(TAG, "Initializing vibrator");
    esp_err_t ret = ESP_OK;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO %d: %s", gpio_num, esp_err_to_name(ret));
        return ret;
    }
    // Set the GPIO to low initially
    ret = gpio_set_level(gpio_num, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set GPIO %d level: %s", gpio_num, esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "Vibrator initialized on GPIO %d", gpio_num);
    vibrator_gpio_num = gpio_num;

    return ret;
}
