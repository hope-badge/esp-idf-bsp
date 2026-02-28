/* HOPE Badge BSP

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdlib.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"

#include "pcf8574.h"

static const char *TAG = "pcf8574";

/**
 * @brief Internal device structure for PCF8574
 */
typedef struct {
    i2c_bus_device_handle_t i2c_dev;    /*!< I2C device handle */
    uint8_t dev_addr;                    /*!< 7-bit I2C address */
    uint8_t output_cache;                /*!< Cached output latch state */
    uint8_t input_mask;                  /*!< Direction mask: 1 = input, 0 = output */
    gpio_num_t int_gpio;                 /*!< Host GPIO for INT pin, or GPIO_NUM_NC */
    pcf8574_int_cb_t int_cb;             /*!< User interrupt callback */
    void *int_cb_arg;                    /*!< User interrupt callback argument */
} pcf8574_device_t;

/* -------------------------------------------------------------------------- */
/*  Internal helpers                                                          */
/* -------------------------------------------------------------------------- */

/**
 * @brief Write the effective output value (cache merged with input mask).
 *
 * Input pins are always driven HIGH (weak pull-up) regardless of cache.
 */
static esp_err_t pcf8574_flush(pcf8574_device_t *device)
{
    uint8_t value = device->output_cache | device->input_mask;
    return i2c_bus_write_byte(device->i2c_dev, NULL_I2C_MEM_ADDR, value);
}

static inline bool pcf8574_pin_valid(uint8_t pin)
{
    return pin < PCF8574_PIN_COUNT;
}

/* -------------------------------------------------------------------------- */
/*  Lifecycle                                                                 */
/* -------------------------------------------------------------------------- */

pcf8574_handle_t pcf8574_create(i2c_bus_handle_t bus, uint8_t dev_addr)
{
    pcf8574_device_t *dev = (pcf8574_device_t *)calloc(1, sizeof(pcf8574_device_t));
    if (dev == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for PCF8574 device");
        return NULL;
    }
    dev->i2c_dev = i2c_bus_device_create(bus, dev_addr, i2c_bus_get_current_clk_speed(bus));
    if (dev->i2c_dev == NULL) {
        ESP_LOGE(TAG, "Failed to create I2C device for PCF8574 at address 0x%02X", dev_addr);
        free(dev);
        return NULL;
    }
    dev->dev_addr = dev_addr;
    dev->output_cache = 0xFF;   /* Power-on default: all pins HIGH */
    dev->input_mask = 0xFF;     /* Assume all pins are inputs initially */
    dev->int_gpio = GPIO_NUM_NC;
    dev->int_cb = NULL;
    dev->int_cb_arg = NULL;

    ESP_LOGD(TAG, "PCF8574 created at address 0x%02X", dev_addr);
    return (pcf8574_handle_t)(dev);
}

esp_err_t pcf8574_delete(pcf8574_handle_t *dev)
{
    if (dev == NULL || *dev == NULL) {
        return ESP_OK;
    }

    pcf8574_device_t *device = (pcf8574_device_t *)(*dev);

    /* Clean up interrupt if registered */
    if (device->int_gpio != GPIO_NUM_NC) {
        pcf8574_unregister_interrupt(*dev);
    }

    i2c_bus_device_delete(&device->i2c_dev);
    free(device);
    *dev = NULL;
    return ESP_OK;
}

/* -------------------------------------------------------------------------- */
/*  Full-byte I/O                                                             */
/* -------------------------------------------------------------------------- */

esp_err_t pcf8574_read(pcf8574_handle_t dev, uint8_t *data)
{
    if (dev == NULL || data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    pcf8574_device_t *device = (pcf8574_device_t *)dev;
    return i2c_bus_read_byte(device->i2c_dev, NULL_I2C_MEM_ADDR, data);
}

esp_err_t pcf8574_write(pcf8574_handle_t dev, uint8_t data)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    pcf8574_device_t *device = (pcf8574_device_t *)dev;
    device->output_cache = data;
    return pcf8574_flush(device);
}

/* -------------------------------------------------------------------------- */
/*  Pin direction                                                             */
/* -------------------------------------------------------------------------- */

esp_err_t pcf8574_set_direction(pcf8574_handle_t dev, uint8_t input_mask)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    pcf8574_device_t *device = (pcf8574_device_t *)dev;
    device->input_mask = input_mask;
    return pcf8574_flush(device);
}

esp_err_t pcf8574_get_direction(pcf8574_handle_t dev, uint8_t *input_mask)
{
    if (dev == NULL || input_mask == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    pcf8574_device_t *device = (pcf8574_device_t *)dev;
    *input_mask = device->input_mask;
    return ESP_OK;
}

/* -------------------------------------------------------------------------- */
/*  Individual pin operations                                                 */
/* -------------------------------------------------------------------------- */

esp_err_t pcf8574_set_pin(pcf8574_handle_t dev, uint8_t pin)
{
    if (dev == NULL || !pcf8574_pin_valid(pin)) {
        return ESP_ERR_INVALID_ARG;
    }

    pcf8574_device_t *device = (pcf8574_device_t *)dev;
    device->output_cache |= (1 << pin);
    return pcf8574_flush(device);
}

esp_err_t pcf8574_clear_pin(pcf8574_handle_t dev, uint8_t pin)
{
    if (dev == NULL || !pcf8574_pin_valid(pin)) {
        return ESP_ERR_INVALID_ARG;
    }

    pcf8574_device_t *device = (pcf8574_device_t *)dev;
    device->output_cache &= ~(1 << pin);
    return pcf8574_flush(device);
}

esp_err_t pcf8574_toggle_pin(pcf8574_handle_t dev, uint8_t pin)
{
    if (dev == NULL || !pcf8574_pin_valid(pin)) {
        return ESP_ERR_INVALID_ARG;
    }

    pcf8574_device_t *device = (pcf8574_device_t *)dev;
    device->output_cache ^= (1 << pin);
    return pcf8574_flush(device);
}

esp_err_t pcf8574_read_pin(pcf8574_handle_t dev, uint8_t pin, uint8_t *level)
{
    if (dev == NULL || level == NULL || !pcf8574_pin_valid(pin)) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t port_data = 0;
    esp_err_t ret = pcf8574_read(dev, &port_data);
    if (ret == ESP_OK) {
        *level = (port_data >> pin) & 0x01;
    }
    return ret;
}

/* -------------------------------------------------------------------------- */
/*  Interrupt support                                                         */
/* -------------------------------------------------------------------------- */

static void IRAM_ATTR pcf8574_isr_handler(void *arg)
{
    pcf8574_device_t *device = (pcf8574_device_t *)arg;
    if (device->int_cb) {
        device->int_cb(device->int_cb_arg);
    }
}

esp_err_t pcf8574_register_interrupt(pcf8574_handle_t dev, gpio_num_t int_gpio_num,
                                     pcf8574_int_cb_t callback, void *arg)
{
    if (dev == NULL || callback == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    pcf8574_device_t *device = (pcf8574_device_t *)dev;

    if (device->int_gpio != GPIO_NUM_NC) {
        ESP_LOGE(TAG, "Interrupt already registered on GPIO %d", device->int_gpio);
        return ESP_ERR_INVALID_STATE;
    }

    /* Configure the host GPIO for falling edge (PCF8574 INT is active-LOW, open-drain) */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << int_gpio_num),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };

    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure INT GPIO %d: %s", int_gpio_num, esp_err_to_name(ret));
        return ret;
    }

    ret = gpio_install_isr_service(0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        /* ESP_ERR_INVALID_STATE means ISR service already installed — that's fine */
        ESP_LOGE(TAG, "Failed to install GPIO ISR service: %s", esp_err_to_name(ret));
        return ret;
    }

    device->int_cb = callback;
    device->int_cb_arg = arg;

    ret = gpio_isr_handler_add(int_gpio_num, pcf8574_isr_handler, device);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add ISR handler for GPIO %d: %s", int_gpio_num, esp_err_to_name(ret));
        device->int_cb = NULL;
        device->int_cb_arg = NULL;
        return ret;
    }

    device->int_gpio = int_gpio_num;
    ESP_LOGD(TAG, "Interrupt registered on GPIO %d", int_gpio_num);
    return ESP_OK;
}

esp_err_t pcf8574_unregister_interrupt(pcf8574_handle_t dev)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    pcf8574_device_t *device = (pcf8574_device_t *)dev;

    if (device->int_gpio == GPIO_NUM_NC) {
        return ESP_ERR_INVALID_STATE;
    }

    gpio_isr_handler_remove(device->int_gpio);
    gpio_reset_pin(device->int_gpio);

    device->int_gpio = GPIO_NUM_NC;
    device->int_cb = NULL;
    device->int_cb_arg = NULL;

    ESP_LOGD(TAG, "Interrupt unregistered");
    return ESP_OK;
}
