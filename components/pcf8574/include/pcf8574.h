/**
 * @file
 * @brief PCF8574 I2C GPIO Expander Driver
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "sdkconfig.h"
#include "i2c_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCF8574_I2C_ADDR_DEFAULT 0x20

/**
 * @brief PCF8574 device handle type (opaque pointer)
 */
typedef void *pcf8574_handle_t;

/**
 * @brief Create and initialize a PCF8574 device on the given I2C bus.
 *
 * @param bus I2C bus handle
 * @param dev_addr 7-bit I2C device address
 * @return pcf8574_handle_t Device handle on success, NULL on failure
 */
pcf8574_handle_t pcf8574_create(i2c_bus_handle_t bus, uint8_t dev_addr);

/**
 * @brief Delete a PCF8574 device and free associated resources.
 *
 * @param[in,out] dev Pointer to the device handle. Will be set to NULL on success.
 * @return
 *      - ESP_OK on success
 */
esp_err_t pcf8574_delete(pcf8574_handle_t *dev);

/**
 * @brief Read the current 8-bit pin state from the PCF8574.
 *
 * @param dev Device handle
 * @param[out] data Pointer to store the read byte
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if dev or data is NULL
 *      - ESP_FAIL on I2C read error
 */
esp_err_t pcf8574_read(pcf8574_handle_t dev, uint8_t *data);

/**
 * @brief Write an 8-bit value to the PCF8574 output pins.
 *
 * @param dev Device handle
 * @param data The byte to write to the output pins
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if dev is NULL
 *      - ESP_FAIL on I2C write error
 */
esp_err_t pcf8574_write(pcf8574_handle_t dev, uint8_t data);

#ifdef __cplusplus
}
#endif
