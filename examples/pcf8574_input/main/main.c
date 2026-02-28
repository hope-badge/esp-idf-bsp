/* PCF8574 Input Reading Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bsp/bsp.h"
#include "pcf8574.h"

static const char *TAG = "pcf8574_input";

/**
 * PCF8574 pin assignment for this example:
 *
 *   P0 – output (directly driven)
 *   P1 – input  (directly read, directly connected to pin/button)
 *   P2 – input  (directly read, directly connected to pin/button)
 *   P3 – input  (directly read, directly connected to pin/button)
 *   P4 – output
 *   P5 – output
 *   P6 – output
 *   P7 – output
 *
 * Input mask: 0x0E  (binary 00001110 → P1, P2, P3 are inputs)
 *
 * For input pins, the PCF8574 uses a weak internal pull-up (~100 µA).
 * Connect buttons/switches between the input pin and GND.
 * When pressed, the pin reads LOW (0). When released, the pull-up holds it HIGH (1).
 */

#define INPUT_MASK  0x0E    /* P1, P2, P3 as inputs */

/*
 * Example 1: Polling — read all inputs every 500 ms
 */
static void pcf8574_polling_task(void *arg)
{
    pcf8574_handle_t dev = (pcf8574_handle_t)arg;
    uint8_t data = 0;
    uint8_t prev_data = 0xFF;

    ESP_LOGI(TAG, "Polling task started — reading PCF8574 inputs every 500 ms");

    while (1) {
        esp_err_t ret = pcf8574_read(dev, &data);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read PCF8574: %s", esp_err_to_name(ret));
        } else {
            /* Only log when something changed */
            if (data != prev_data) {
                ESP_LOGI(TAG, "Port read: 0x%02X (bin: %c%c%c%c%c%c%c%c)",
                         data,
                         (data & 0x80) ? '1' : '0',
                         (data & 0x40) ? '1' : '0',
                         (data & 0x20) ? '1' : '0',
                         (data & 0x10) ? '1' : '0',
                         (data & 0x08) ? '1' : '0',
                         (data & 0x04) ? '1' : '0',
                         (data & 0x02) ? '1' : '0',
                         (data & 0x01) ? '1' : '0');

                /* Read individual input pins */
                for (uint8_t pin = 1; pin <= 3; pin++) {
                    uint8_t level = 0;
                    pcf8574_read_pin(dev, pin, &level);
                    ESP_LOGI(TAG, "  P%d = %d %s", pin, level,
                             level == 0 ? "(pressed)" : "(released)");
                }
                prev_data = data;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/*
 * Example 2: Interrupt-driven — react immediately to pin changes
 *
 * The PCF8574 INT pin is active-LOW and fires on any input change.
 * This ISR callback notifies a task that performs the actual I2C read.
 */
static TaskHandle_t int_task_handle = NULL;

static void IRAM_ATTR pcf8574_int_callback(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (int_task_handle) {
        vTaskNotifyGiveFromISR(int_task_handle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static void pcf8574_interrupt_task(void *arg)
{
    pcf8574_handle_t dev = (pcf8574_handle_t)arg;
    uint8_t data = 0;

    ESP_LOGI(TAG, "Interrupt task started — waiting for PCF8574 pin changes");

    while (1) {
        /* Block until the ISR signals a pin change */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        esp_err_t ret = pcf8574_read(dev, &data);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "[INT] Port read: 0x%02X", data);
            for (uint8_t pin = 1; pin <= 3; pin++) {
                uint8_t level = (data >> pin) & 0x01;
                ESP_LOGI(TAG, "[INT]   P%d = %d %s", pin, level,
                         level == 0 ? "(pressed)" : "(released)");
            }
        } else {
            ESP_LOGE(TAG, "[INT] Failed to read PCF8574: %s", esp_err_to_name(ret));
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "PCF8574 Input Reading Example");

    /* Initialize the BSP (sets up I2C bus, etc.) */
    ESP_ERROR_CHECK(bsp_init());

    /*
     * Create the PCF8574 device.
     * The BSP already creates one internally, but here we show standalone usage.
     */
    extern i2c_bus_handle_t i2c_bus; /* Exposed by the BSP for reuse */

    pcf8574_handle_t pcf_dev = pcf8574_create(i2c_bus, PCF8574_I2C_ADDR_DEFAULT);
    if (pcf_dev == NULL) {
        ESP_LOGE(TAG, "Failed to create PCF8574 device");
        return;
    }

    /*
     * Configure pin directions:
     *   INPUT_MASK = 0x0E → P1, P2, P3 are inputs (held HIGH by weak pull-up)
     *   Remaining pins are outputs.
     */
    ESP_ERROR_CHECK(pcf8574_set_direction(pcf_dev, INPUT_MASK));
    ESP_LOGI(TAG, "Direction set: input_mask=0x%02X (P1, P2, P3 = input)", INPUT_MASK);

    /* Set output pin P0 HIGH as a demo */
    ESP_ERROR_CHECK(pcf8574_set_pin(pcf_dev, 0));
    ESP_LOGI(TAG, "P0 set HIGH");

    /*
     * Choose one of the two approaches below.
     * Uncomment EXAMPLE_USE_INTERRUPT to use interrupt mode instead of polling.
     */
/* #define EXAMPLE_USE_INTERRUPT */

#ifdef EXAMPLE_USE_INTERRUPT
    /*
     * Interrupt-driven approach
     *
     * Connect the PCF8574 INT pin to a host GPIO. The BSP does not define
     * a default INT pin, so set this to your wiring. For example, GPIO 4:
     */
    #define PCF8574_INT_GPIO  GPIO_NUM_4

    /* Create the task first so the handle is valid before the ISR fires */
    xTaskCreate(pcf8574_interrupt_task, "pcf8574_int", 3072, pcf_dev, 10, &int_task_handle);

    ESP_ERROR_CHECK(pcf8574_register_interrupt(pcf_dev, PCF8574_INT_GPIO,
                                               pcf8574_int_callback, NULL));
    ESP_LOGI(TAG, "Interrupt registered on GPIO %d — press buttons to see events", PCF8574_INT_GPIO);
#else
    /*
     * Polling approach — read inputs periodically
     */
    xTaskCreate(pcf8574_polling_task, "pcf8574_poll", 3072, pcf_dev, 5, NULL);
#endif

    ESP_LOGI(TAG, "Example running. Press buttons connected to P1, P2, P3.");
}
