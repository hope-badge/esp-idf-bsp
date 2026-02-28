#pragma once

#include "sdkconfig.h"
#include "driver/gpio.h"

#include "iot_button.h"
#include "led_strip.h"
#include "max17048.h"
#include "pcf8574.h"

/**************************************************************************************************
 *  BSP Capabilities
 **************************************************************************************************/

/**************************************************************************************************
 *  Pinout
 **************************************************************************************************/
/* I2C */
#define BSP_I2C_SCL             (CONFIG_BSP_I2C_GPIO_SCL)
#define BSP_I2C_SDA             (CONFIG_BSP_I2C_GPIO_SDA)
#define BSP_I2C_NUM             (CONFIG_BSP_I2C_NUM)

/* Buttons */
#define BSP_BUTTON_1_GPIO       (CONFIG_BSP_BUTTON_1_GPIO)
#define BSP_BUTTON_2_GPIO       (CONFIG_BSP_BUTTON_2_GPIO)
#define BSP_BUTTON_3_GPIO       (CONFIG_BSP_BUTTON_3_GPIO)
#define BSP_BUTTON_4_GPIO       (CONFIG_BSP_BUTTON_4_GPIO)

/* Buttons GPIO index */
#define BSP_BUTTON_NUM          (4)
#define BSP_BUTTON_1_GPIO_INDEX (0)
#define BSP_BUTTON_2_GPIO_INDEX (1)
#define BSP_BUTTON_3_GPIO_INDEX (2)
#define BSP_BUTTON_4_GPIO_INDEX (3)

/* Buttons GPIO active level */
#define BSP_BUTTON_1_ACTIVE_LEVEL (CONFIG_BSP_BUTTON_1_LEVEL)
#define BSP_BUTTON_2_ACTIVE_LEVEL (CONFIG_BSP_BUTTON_2_LEVEL)
#define BSP_BUTTON_3_ACTIVE_LEVEL (CONFIG_BSP_BUTTON_3_LEVEL)
#define BSP_BUTTON_4_ACTIVE_LEVEL (CONFIG_BSP_BUTTON_4_LEVEL)

/* Leds */
#define BSP_LED_IO              (CONFIG_BSP_LED_GPIO)
#define BSP_LED_RGB_IO          (CONFIG_BSP_LED_RGB_GPIO)
#define BSP_LED_RGB_PIXELS      (CONFIG_BSP_LED_RGB_PIXELS_NUM)

/* Vibramotor */
#define BSP_VIBRAMOTOR_IO       (CONFIG_BSP_VIBRATION_MOTOR_GPIO)

/* IrDA */
#define BSP_IRDA_TX_IO          (CONFIG_BSP_IRDA_TX_GPIO)
#define BSP_IRDA_RX_IO          (CONFIG_BSP_IRDA_RX_GPIO)

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 *
 * BSP init and deinit
 *
 **************************************************************************************************/

/**
 * @brief Initialize BSP
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   BSP parameter error
 *      - ESP_FAIL              BSP initialization error
 *
 * @note This function initializes the I2C driver, buttons, LEDs, and other BSP components.
 */
 esp_err_t bsp_init(void);

/**************************************************************************************************
 *
 * GPIO
 *
 **************************************************************************************************/

/**
 * @brief Set GPIO level
 *
 * @param gpio_num GPIO number
 * @param level GPIO level (0 or 1)
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   GPIO parameter error
 */
esp_err_t bsp_gpio_set_level(gpio_num_t gpio_num, uint32_t level);

/**
 * @brief Get GPIO level
 *
 * @param gpio_num GPIO number
 *
 * @return
 *      - GPIO level (0 or 1) on success
 *      - ESP_ERR_INVALID_ARG if the GPIO number is invalid
 */
int8_t bsp_gpio_get_level(gpio_num_t gpio_num);

/**************************************************************************************************
 *
 * I2C interface
 *
 **************************************************************************************************/

/**
 * @brief Init I2C driver
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   I2C parameter error
 *      - ESP_FAIL              I2C driver installation error
 *
 */
esp_err_t bsp_i2c_init(void);

/**
 * @brief Deinit I2C driver and free its resources
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   I2C parameter error
 *
 */
esp_err_t bsp_i2c_deinit(void);

/**************************************************************************************************
 *
 * Button
 *
 **************************************************************************************************/

/**
 * @brief Initialize buttons
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   Button parameter error
 *      - ESP_FAIL              Button initialization error
 */
esp_err_t bsp_buttons_init(void);

/**
 * @brief Register button callbacks
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   Button parameter error
 *      - ESP_FAIL              Button callback registration error
 */
esp_err_t bsp_register_button_callbacks(void);

/**
 * @brief Get button handle
 *
 * @param btn_num Button number (0 to BSP_BUTTON_NUM - 1)
 *
 * @return
 *      - Button handle if successful
 *      - NULL if the button number is invalid or the button is not initialized
 */
button_handle_t bsp_get_button_handle(uint8_t btn_num);

/**************************************************************************************************
 *
 * LEDs
 *
 **************************************************************************************************/

/**
 * @brief Initialize the LED
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   LED parameter error
 *      - ESP_FAIL              LED initialization error
 */
esp_err_t bsp_led_init(void);

/**
 * @brief Initialize the RGB LED
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   RGB LED parameter error
 *      - ESP_FAIL              RGB LED initialization error
 */
esp_err_t bsp_init_led_rgb(void);

/**
 * @brief Get LED handle
 *
 * @return
 *      - LED handle
 */

led_strip_handle_t bsp_get_led_rgb_handle(void);

/**************************************************************************************************
 *
 * Fuel Gauge
 *
 **************************************************************************************************/

/**
 * @brief Initialize the fuel gauge
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   Fuel gauge parameter error
 *      - ESP_FAIL              Fuel gauge initialization error
 */
esp_err_t bsp_fuel_gauge_init(void);

/**
 * @brief Get the battery percentage
 *
 * @return
 *      - Battery percentage as a float value (0.0 to 100.0)
 *      - Returns -1.0f on error
 */
float bsp_get_battery_percentage(void);

/**
 * @brief Get the battery voltage
 *
 * @return
 *      - Battery voltage as a float value (in volts)
 *      - Returns -1.0f on error
 */
float bsp_get_battery_voltage(void);

/**************************************************************************************************
 *
 * PCF8574
 *
 **************************************************************************************************/

/**
 * @brief Initialize the PCF8574 I/O expander
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   PCF8574 parameter error
 *      - ESP_FAIL              PCF8574 initialization error
 */
esp_err_t bsp_pcf8574_init(void);

/**
 * @brief Get the PCF8574 device handle
 *
 * @return
 *      - PCF8574 handle if initialized
 *      - NULL if not initialized
 */
pcf8574_handle_t bsp_pcf8574_get_handle(void);

/**
 * @brief Read the state of the PCF8574 I/O expander
 *
 * @param data Pointer to store the read data
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   PCF8574 handle or data pointer is NULL
 *      - ESP_FAIL              Read operation failed
 */
esp_err_t bsp_pcf8574_read_ios(uint8_t *data);

#ifdef __cplusplus
}
#endif
