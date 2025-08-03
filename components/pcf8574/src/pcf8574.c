/* HOPE Badge BSP

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "pcf8574.h"
#include <stdbool.h>
#include <stdint.h>

pcf8574_handle_t pcf8574_create(i2c_bus_handle_t bus, uint8_t dev_addr)
{
    pcf8574_device_t *dev = (pcf8574_device_t *)calloc(1, sizeof(pcf8574_device_t));
    dev->i2c_dev = i2c_bus_device_create(bus, dev_addr, i2c_bus_get_current_clk_speed(bus));
    if (dev->i2c_dev == NULL) {
        free(dev);
        return NULL;
    }
    dev->dev_addr = dev_addr;
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

    uint8_t buf = 0x00;
    esp_err_t ret = i2c_bus_read_bytes(device->i2c_dev, 0x00, 1, &buf);  // 0x00 is a dummy register
    if (ret != ESP_OK) {
        return ESP_FAIL;
    }

    *data = buf;
    return ret;
}

esp_err_t pcf8574_write(pcf8574_handle_t dev, uint8_t data)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    pcf8574_device_t *device = (pcf8574_device_t *)dev;
    return i2c_bus_write_byte(device->i2c_dev, 0x00, data);
}
