/**
 * @file
 * @brief ESP BSP: Generic
 */

#pragma once

#include "sdkconfig.h"
#include "i2c_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCF8574_I2C_ADDR_DEFAULT 0x20

typedef void *pcf8574_handle_t;
typedef struct {
    i2c_bus_device_handle_t i2c_dev;
    uint8_t dev_addr;
} pcf8574_device_t;

esp_err_t pcf8574_read(pcf8574_handle_t dev, uint8_t *data);
esp_err_t pcf8574_write(pcf8574_handle_t dev, uint8_t data);
pcf8574_handle_t pcf8574_create(i2c_bus_handle_t bus, uint8_t dev_addr);
esp_err_t pcf8574_delete(pcf8574_handle_t *dev);

#ifdef __cplusplus
}
#endif
