/* HOPE Badge BSP

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "vibramotor.h"

static const char *TAG = "Vibramotor";
static int8_t vibramotor_gpio_num = -1;

static TaskHandle_t vibramotor_task_handle = NULL;

static void vibramotor_run_task(void *pvParameters)
{
    vibramotor_params_t params;
    memcpy(&params, pvParameters, sizeof(vibramotor_params_t));

    // Free the heap-allocated parameters now that we have a local copy
    free(pvParameters);

    uint32_t on_time = params.time_on_ms;
    uint32_t off_time = params.time_off_ms;
    uint32_t repeat_count = params.repeat_count;

    for (uint32_t i = 0; i < repeat_count; i++) {
        gpio_set_level(vibramotor_gpio_num, 1);
        vTaskDelay(pdMS_TO_TICKS(on_time));
        gpio_set_level(vibramotor_gpio_num, 0);
        vTaskDelay(pdMS_TO_TICKS(off_time));
    }

    // Ensure motor is off and clear handle before self-deleting
    gpio_set_level(vibramotor_gpio_num, 0);
    vibramotor_task_handle = NULL;
    vTaskDelete(NULL);
}

void vibramotor_stop(void)
{
    if (vibramotor_task_handle != NULL) {
        vTaskDelete(vibramotor_task_handle);
        vibramotor_task_handle = NULL;
    }

    // Ensure motor is off regardless
    if (vibramotor_gpio_num >= 0) {
        gpio_set_level(vibramotor_gpio_num, 0);
    }
}

esp_err_t vibramotor_run(uint16_t time_on_ms, uint16_t time_off_ms, uint16_t cycles)
{
    if (vibramotor_gpio_num == -1) {
        ESP_LOGE(TAG, "Vibramotor GPIO not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Stop any currently running vibration before starting a new one
    vibramotor_stop();

    // Heap-allocate params so the task can safely read them after this function returns
    vibramotor_params_t *params = malloc(sizeof(vibramotor_params_t));
    if (params == NULL) {
        ESP_LOGE(TAG, "Failed to allocate vibramotor params");
        return ESP_ERR_NO_MEM;
    }

    params->time_on_ms = time_on_ms;
    params->time_off_ms = time_off_ms;
    params->repeat_count = cycles;

    BaseType_t xret = xTaskCreate(vibramotor_run_task, "vibramotor_task", 2048,
                                  params, 5, &vibramotor_task_handle);
    if (xret != pdPASS) {
        free(params);
        ESP_LOGE(TAG, "Failed to create vibramotor task");
        return ESP_FAIL;
    }

    return ESP_OK;
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

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO %d: %s", gpio_num, esp_err_to_name(ret));
        return ret;
    }

    ret = gpio_set_level(gpio_num, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set GPIO %d level: %s", gpio_num, esp_err_to_name(ret));
        return ret;
    }

    vibramotor_gpio_num = gpio_num;
    ESP_LOGI(TAG, "Vibramotor initialized on GPIO %d", gpio_num);

    return ESP_OK;
}
