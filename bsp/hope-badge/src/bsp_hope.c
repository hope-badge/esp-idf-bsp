/* HOPE Badge BSP

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdbool.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"

#include "bsp/bsp_hope.h"
#include "bsp_err_check.h"
#include "button_gpio.h"

static const char *TAG = "BSP-HOPE";

#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)
#define LED_STRIP_MEMORY_BLOCK_WORDS 0

static i2c_bus_handle_t i2c_bus = NULL;
static bool i2c_initialized = false;
static button_handle_t btn[BSP_BUTTON_NUM] = {NULL};
static led_strip_handle_t led_rgb_handle = NULL;
static max17048_handle_t max17048 = NULL;
static pcf8574_handle_t pcf_dev = NULL;

esp_err_t bsp_i2c_init(void)
{
    if (i2c_initialized) {
        ESP_LOGW(TAG, "I2C bus is already initialized");
        return ESP_OK;
    }

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = BSP_I2C_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = BSP_I2C_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = CONFIG_BSP_I2C_CLK_SPEED_HZ,
    };

    i2c_bus = i2c_bus_create(BSP_I2C_NUM, &conf);
    if (i2c_bus == NULL) {
        ESP_LOGE(TAG, "Failed to create I2C bus");
        return ESP_FAIL;
    }

    i2c_initialized = true;

    return ESP_OK;
}

esp_err_t bsp_i2c_deinit(void)
{
    // Check if I2C bus is initialized
    if (!i2c_initialized) {
        ESP_LOGW(TAG, "I2C bus is not initialized, nothing to deinitialize");
        return ESP_OK;
    }
    // Check if I2C bus handle is valid
    if (i2c_bus == NULL) {
        ESP_LOGE(TAG, "I2C bus handle is NULL, cannot deinitialize");
        return ESP_ERR_INVALID_STATE;
    }
    // Delete the I2C bus
    esp_err_t ret = i2c_bus_delete(i2c_bus);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to delete I2C bus: %s", esp_err_to_name(ret));
        return ret;
    }
    // Set the I2C bus handle to NULL
    i2c_bus = NULL;
    // Reset the I2C initialized flag
    i2c_initialized = false;
    return ESP_OK;
}

button_handle_t bsp_get_button_handle(uint8_t btn_num)
{
    if (btn_num >= BSP_BUTTON_NUM) {
        return NULL;
    }
    return btn[btn_num];
}

esp_err_t bsp_buttons_init(void)
{
    // Initialize button 1
    button_config_t btn_1_cfg = {0};
    button_gpio_config_t btn_1_gpio_cfg = {
        .gpio_num = BSP_BUTTON_1_GPIO,
        .active_level = BSP_BUTTON_1_ACTIVE_LEVEL,
        .enable_power_save = true,
    };

    btn[0] = NULL;
    esp_err_t ret = iot_button_new_gpio_device(&btn_1_cfg, &btn_1_gpio_cfg, &btn[0]);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize button 1: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize button 2
    button_config_t btn_2_cfg = {0};
    button_gpio_config_t btn_2_gpio_cfg = {
        .gpio_num = BSP_BUTTON_2_GPIO,
        .active_level = BSP_BUTTON_2_ACTIVE_LEVEL,
        .enable_power_save = true,
    };

    btn[1] = NULL;
    ret = iot_button_new_gpio_device(&btn_2_cfg, &btn_2_gpio_cfg, &btn[1]);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize button 2: %s", esp_err_to_name(ret));
        return ret;
    }

    // Uncomment the following lines if you are not using USB

    /*
    button_config_t btn_3_cfg = {0};
    button_gpio_config_t btn_3_gpio_cfg = {
        .gpio_num = BSP_BUTTON_3_GPIO,
        .active_level = BSP_BUTTON_3_ACTIVE_LEVEL,
        .enable_power_save = true,
    };

    btn[2] = NULL;
    ret = iot_button_new_gpio_device(&btn_3_cfg, &btn_3_gpio_cfg, &btn[2]);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize button 3: %s", esp_err_to_name(ret));
        return ret;
    }

    button_config_t btn_4_cfg = {0};
    button_gpio_config_t btn_4_gpio_cfg = {
        .gpio_num = BSP_BUTTON_4_GPIO,
        .active_level = BSP_BUTTON_4_ACTIVE_LEVEL,
        .enable_power_save = true,
    };

    btn[3] = NULL;
    ret = iot_button_new_gpio_device(&btn_4_cfg, &btn_4_gpio_cfg, &btn[3]);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize button 4: %s", esp_err_to_name(ret));
        return ret;
    }
    */

    return ret;
}

esp_err_t bsp_led_init(void)
{
    // Initialize the LED GPIO
    gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << BSP_LED_IO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    BSP_ERROR_CHECK_RETURN_ERR(gpio_config(&led_config));

    // Set the LED to off initially
    BSP_ERROR_CHECK_RETURN_ERR(gpio_set_level(BSP_LED_IO, 0));

    return ESP_OK;
}

esp_err_t bsp_gpio_set_level(gpio_num_t gpio_num, uint32_t level)
{
    // Check if the GPIO number is valid
    if (gpio_num < 0 || gpio_num >= GPIO_NUM_MAX) {
        ESP_LOGE(TAG, "Invalid GPIO number: %d", gpio_num);
        return ESP_ERR_INVALID_ARG;
    }

    // Set the GPIO level
    esp_err_t ret = gpio_set_level(gpio_num, level);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set GPIO level: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

int8_t bsp_gpio_get_level(gpio_num_t gpio_num)
{
    // Check if the GPIO number is valid
    if (gpio_num < 0 || gpio_num >= GPIO_NUM_MAX) {
        ESP_LOGE(TAG, "Invalid GPIO number: %d", gpio_num);
        return -1; // Return -1 for invalid GPIO number
    }

    // Get the GPIO level
    int8_t level = gpio_get_level(gpio_num);

    return level;
}

led_strip_handle_t bsp_get_led_rgb_handle(void)
{
    if (led_rgb_handle == NULL) {
        ESP_LOGE(TAG, "RGB LED handle is not initialized");
        return NULL;
    }
    return led_rgb_handle;
}

esp_err_t bsp_init_led_rgb(void)
{

    // LED strip general initialization, according to your led board design
    led_strip_config_t strip_config = {
        .strip_gpio_num = BSP_LED_RGB_IO, // The GPIO that connected to the LED strip's data line
        .max_leds = BSP_LED_RGB_PIXELS,      // The number of LEDs in the strip,
        .led_model = LED_MODEL_WS2812,        // LED strip model
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // The color order of the strip: GRB
        .flags = {
            .invert_out = false, // don't invert the output signal
        }
    };

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
        .resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT counter clock frequency
        .mem_block_symbols = LED_STRIP_MEMORY_BLOCK_WORDS, // the memory block size used by the RMT channel
        .flags = {
            .with_dma = false,     // Using DMA can improve performance when driving more LEDs
        }
    };

    esp_err_t ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_rgb_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LED strip RMT device: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");

    return ESP_OK;
}

float bsp_get_battery_voltage(void)
{
    if (max17048 == NULL) {
        ESP_LOGE(TAG, "MAX17048 fuel gauge handle is not initialized");
        return -1.0f; // Return an error value
    }

    float voltage = 0;
    esp_err_t ret = max17048_get_cell_voltage(max17048, &voltage);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get battery voltage: %s", esp_err_to_name(ret));
        return -1.0f; // Return an error value
    }

    return voltage;
}

float bsp_get_battery_percentage(void)
{
    if (max17048 == NULL) {
        ESP_LOGE(TAG, "MAX17048 fuel gauge handle is not initialized");
        return -1.0f; // Return an error value
    }

    float percent = 0;
    esp_err_t ret = max17048_get_cell_percent(max17048, &percent);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get battery percentage: %s", esp_err_to_name(ret));
        return -1.0f; // Return an error value
    }

    return percent;
}

esp_err_t bsp_fuel_gauge_init(void)
{
    // Initialize the MAX17048 fuel gauge
    max17048 = max17048_create(i2c_bus, MAX17048_I2C_ADDR_DEFAULT);

    if (max17048 == NULL) {
        ESP_LOGE(TAG, "Failed to create MAX17048 fuel gauge handle");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t bsp_pcf8574_init(void)
{
    // Try default PCF8574 address (0x20) first, then PCF8574A address (0x38) as fallback
    pcf_dev = pcf8574_create(i2c_bus, PCF8574_I2C_ADDR_DEFAULT);
    if (pcf_dev == NULL) {
        ESP_LOGW(TAG, "PCF8574 not found at 0x%02X, trying PCF8574A at 0x%02X",
                 PCF8574_I2C_ADDR_DEFAULT, PCF8574A_I2C_ADDR_DEFAULT);
        pcf_dev = pcf8574_create(i2c_bus, PCF8574A_I2C_ADDR_DEFAULT);
        if (pcf_dev == NULL) {
            ESP_LOGE(TAG, "Failed to create PCF8574 handle at either address");
            return ESP_FAIL;
        }
    }

    // Set direction: P1, P2, P3 as inputs (weak pull-up), rest as outputs
    uint8_t io_dir_mask = 0x0E;  // 00001110 -> P3, P2, P1 = input
    esp_err_t ret = pcf8574_set_direction(pcf_dev, io_dir_mask);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set PCF8574 direction: %s", esp_err_to_name(ret));
        pcf_dev = NULL;
        return ret;
    }

    ESP_LOGI(TAG, "PCF8574 initialized successfully");
    return ESP_OK;
}

pcf8574_handle_t bsp_pcf8574_get_handle(void)
{
    if (pcf_dev == NULL) {
        ESP_LOGE(TAG, "PCF8574 handle is not initialized");
    }
    return pcf_dev;
}

esp_err_t bsp_register_button_callbacks(void)
{
    ESP_LOGW(TAG, "No button callbacks registered (not implemented)");
    return ESP_OK;
}

esp_err_t bsp_pcf8574_read_ios(uint8_t *data)
{
    if (pcf_dev == NULL) {
        ESP_LOGE(TAG, "PCF8574 handle is not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    if (data == NULL) {
        ESP_LOGE(TAG, "Data pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    return pcf8574_read(pcf_dev, data);
}

esp_err_t bsp_init(void)
{
    ESP_LOGI(TAG, "Initializing Hope Badge BSP");
    esp_err_t ret = ESP_OK;
    esp_err_t err;

    // Initialize I2C — required by fuel gauge and PCF8574
    err = bsp_i2c_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C: %s", esp_err_to_name(err));
        return err;  // Cannot continue without I2C
    }

    // Initialize buttons
    err = bsp_buttons_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize buttons: %s", esp_err_to_name(err));
        if (ret == ESP_OK) ret = err;
    }

    // Initialize LED
    err = bsp_led_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LED: %s", esp_err_to_name(err));
        if (ret == ESP_OK) ret = err;
    }

    // Initialize RGB LED
    err = bsp_init_led_rgb();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize RGB LED: %s", esp_err_to_name(err));
        if (ret == ESP_OK) ret = err;
    }

    // Initialize fuel gauge
    err = bsp_fuel_gauge_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize fuel gauge: %s", esp_err_to_name(err));
        if (ret == ESP_OK) ret = err;
    }

    // Initialize PCF8574 I/O expander (non-critical)
    err = bsp_pcf8574_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to initialize PCF8574: %s", esp_err_to_name(err));
    }

    ESP_LOGI(TAG, "BSP initialization complete");

    return ret;
}
