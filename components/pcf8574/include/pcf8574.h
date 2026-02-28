/**
 * @file
 * @brief PCF8574 I2C GPIO Expander Driver
 *
 * Driver for the NXP PCF8574/PCF8574A 8-bit I2C I/O expander.
 * Supports full byte and individual pin read/write, pin direction
 * configuration, and interrupt-driven pin change notification.
 *
 * The PCF8574 has quasi-bidirectional I/O: pins written HIGH have a
 * weak internal pull-up (~100 µA) and can be used as inputs.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "i2c_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCF8574_I2C_ADDR_DEFAULT    0x20    /*!< Default I2C address for PCF8574 (A2=A1=A0=0) */
#define PCF8574A_I2C_ADDR_DEFAULT   0x38    /*!< Default I2C address for PCF8574A (A2=A1=A0=0) */
#define PCF8574_PIN_COUNT           8       /*!< Number of I/O pins on the PCF8574 */

/**
 * @brief PCF8574 device handle type (opaque pointer)
 */
typedef void *pcf8574_handle_t;

/**
 * @brief Callback type for PCF8574 interrupt events.
 *
 * Called from ISR context when the INT pin fires. Keep the handler short
 * and defer heavy work to a task (e.g., via xTaskNotifyFromISR).
 *
 * @param arg User-supplied argument passed during registration
 */
typedef void (*pcf8574_int_cb_t)(void *arg);

/*******************************************************************************
 * Lifecycle
 ******************************************************************************/

/**
 * @brief Create and initialize a PCF8574 device on the given I2C bus.
 *
 * All pins default to HIGH (input-ready with weak pull-up) after creation,
 * matching the PCF8574 power-on state.
 *
 * @param bus I2C bus handle
 * @param dev_addr 7-bit I2C device address
 * @return pcf8574_handle_t Device handle on success, NULL on failure
 */
pcf8574_handle_t pcf8574_create(i2c_bus_handle_t bus, uint8_t dev_addr);

/**
 * @brief Delete a PCF8574 device and free associated resources.
 *
 * If an interrupt was registered, it will be unregistered automatically.
 *
 * @param[in,out] dev Pointer to the device handle. Will be set to NULL on success.
 * @return
 *      - ESP_OK on success
 */
esp_err_t pcf8574_delete(pcf8574_handle_t *dev);

/*******************************************************************************
 * Full-byte I/O
 ******************************************************************************/

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
 * Updates the internal output cache. Input pins (direction mask = 1)
 * are forced HIGH automatically.
 *
 * @param dev Device handle
 * @param data The byte to write to the output pins
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if dev is NULL
 *      - ESP_FAIL on I2C write error
 */
esp_err_t pcf8574_write(pcf8574_handle_t dev, uint8_t data);

/*******************************************************************************
 * Pin direction
 ******************************************************************************/

/**
 * @brief Set the I/O direction for all 8 pins.
 *
 * Bits set to 1 in @p input_mask are configured as inputs (written HIGH
 * to enable the weak pull-up). Bits set to 0 are configured as outputs.
 * Output pins retain their last written value.
 *
 * @param dev Device handle
 * @param input_mask Bitmask where 1 = input, 0 = output (e.g., 0x0F = P0-P3 input)
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if dev is NULL
 *      - ESP_FAIL on I2C write error
 */
esp_err_t pcf8574_set_direction(pcf8574_handle_t dev, uint8_t input_mask);

/**
 * @brief Get the current direction mask.
 *
 * @param dev Device handle
 * @param[out] input_mask Pointer to store current direction mask (1 = input)
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if dev or input_mask is NULL
 */
esp_err_t pcf8574_get_direction(pcf8574_handle_t dev, uint8_t *input_mask);

/*******************************************************************************
 * Individual pin operations
 ******************************************************************************/

/**
 * @brief Set a single pin HIGH.
 *
 * Uses the cached output state to avoid an I2C read.
 *
 * @param dev Device handle
 * @param pin Pin number (0–7)
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if dev is NULL or pin > 7
 *      - ESP_FAIL on I2C write error
 */
esp_err_t pcf8574_set_pin(pcf8574_handle_t dev, uint8_t pin);

/**
 * @brief Set a single pin LOW.
 *
 * Uses the cached output state to avoid an I2C read.
 *
 * @param dev Device handle
 * @param pin Pin number (0–7)
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if dev is NULL or pin > 7
 *      - ESP_FAIL on I2C write error
 */
esp_err_t pcf8574_clear_pin(pcf8574_handle_t dev, uint8_t pin);

/**
 * @brief Toggle a single pin.
 *
 * Uses the cached output state to avoid an I2C read.
 *
 * @param dev Device handle
 * @param pin Pin number (0–7)
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if dev is NULL or pin > 7
 *      - ESP_FAIL on I2C write error
 */
esp_err_t pcf8574_toggle_pin(pcf8574_handle_t dev, uint8_t pin);

/**
 * @brief Read the level of a single pin.
 *
 * Performs an I2C read to get the actual pin state.
 *
 * @param dev Device handle
 * @param pin Pin number (0–7)
 * @param[out] level Pointer to store the pin level (0 or 1)
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if dev or level is NULL, or pin > 7
 *      - ESP_FAIL on I2C read error
 */
esp_err_t pcf8574_read_pin(pcf8574_handle_t dev, uint8_t pin, uint8_t *level);

/*******************************************************************************
 * Interrupt support
 ******************************************************************************/

/**
 * @brief Register an interrupt handler for PCF8574 pin-change events.
 *
 * The PCF8574 INT pin is active-LOW, open-drain. This function configures
 * the specified host GPIO with a falling-edge ISR that calls @p callback.
 *
 * @note Only one interrupt can be registered per device. Call
 *       pcf8574_unregister_interrupt() before registering a new one.
 *
 * @param dev Device handle
 * @param int_gpio_num Host GPIO connected to the PCF8574 INT pin
 * @param callback Function to call on interrupt (runs in ISR context)
 * @param arg User argument passed to the callback
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if dev or callback is NULL
 *      - ESP_ERR_INVALID_STATE if an interrupt is already registered
 *      - ESP_FAIL on GPIO configuration error
 */
esp_err_t pcf8574_register_interrupt(pcf8574_handle_t dev, gpio_num_t int_gpio_num,
                                     pcf8574_int_cb_t callback, void *arg);

/**
 * @brief Unregister the interrupt handler and release the host GPIO ISR.
 *
 * @param dev Device handle
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG if dev is NULL
 *      - ESP_ERR_INVALID_STATE if no interrupt was registered
 */
esp_err_t pcf8574_unregister_interrupt(pcf8574_handle_t dev);

#ifdef __cplusplus
}
#endif
