/* HOPE Badge BSP

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

#include "pcf8574.h"

static const char *TAG = "pcf8574";

/**
 * @brief Internal device structure for PCF8574
 */
typedef struct {
    i2c_bus_device_handle_t i2c_dev;
    uint8_t dev_addr;
} pcf8574_device_t;

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
    ESP_LOGD(TAG, "PCF8574 created at address 0x%02X", dev_addr);
    return (pcf8574_handle_t)(dev);
}

esp_err_t pcf8574_delete(pcf8574_handle_t *dev)
{
    if (*dev == NULL) {
        return ESP_OK;
    }

    pcf8574_device_t *device = (pcf8574_device_t *)(*dev);
    i2c_bus_device_delete(&device->i2c_dev);
    free(device);
    *dev = NULL;
    return ESP_OK;
}

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
    return i2c_bus_write_byte(device->i2c_dev, NULL_I2C_MEM_ADDR, data);
}
