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

#include "vibramotor.h"
#include <stdbool.h>
#include <stdint.h>

static const char *TAG = "Vibramotor";
static int8_t vibramotor_gpio_num = -1;

void vibramotor_run_task(void *pvParameters)
{
    vibramotor_params_t *params = (vibramotor_params_t *) pvParameters;

    if (params == NULL) {
        ESP_LOGE(TAG, "Invalid parameters for vibramotor task");
        vTaskDelete(NULL);
        return;
    }
    
    uint32_t on_time = params->time_on_ms;
    uint32_t off_time = params->time_off_ms;
    uint32_t repeat_count = params->repeat_count;

    for (uint32_t i = 0; i < repeat_count; i++) {
        gpio_set_level(vibramotor_gpio_num, 1);
        vTaskDelay(on_time / portTICK_PERIOD_MS);
        gpio_set_level(vibramotor_gpio_num, 0);
        vTaskDelay(off_time / portTICK_PERIOD_MS);
    }
    // Optionally delete task after completion
    vTaskDelete(NULL);
}


esp_err_t vibramotor_stop(void)
{
    esp_err_t ret = ESP_OK;
    return ret;
}

esp_err_t vibramotor_run(uint16_t time_on_ms, uint16_t time_off_ms, uint16_t cycles)
{
    esp_err_t ret = ESP_OK;

    vibramotor_params_t params = {
        .time_on_ms = time_on_ms,
        .time_off_ms = time_off_ms,
        .repeat_count = cycles,
    };

    // Create a task to run the vibramotor
    if (vibramotor_gpio_num == -1) {
        ESP_LOGE(TAG, "Vibramotor GPIO not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ret = xTaskCreate(vibramotor_run_task, "vibramotor_task", 2048, &params, 5, NULL);
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create vibramotor task: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    return ret;
}

/**
 * @brief Initialize the vibramotor by configuring the GPIO pin.
 *
 * @param gpio_num The GPIO number to which the vibramotor is connected.
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if the GPIO number is invalid
 *      - Other error codes on failure to configure the GPIO
 */

esp_err_t vibramotor_init(uint8_t gpio_num)
{
    ESP_LOGI(TAG, "Initializing vibramotor");
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
    ESP_LOGI(TAG, "Vibramotor initialized on GPIO %d", gpio_num);
    vibramotor_gpio_num = gpio_num;

    return ret;
}
