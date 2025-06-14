# HOPE Badge Board Support Package (BSP)

[![Component Registry](https://components.espressif.com/components/hope-badge/hope-badge/badge.svg)](https://components.espressif.com/components/hope-badge/hope-badge)

## Overview

This is the HOPE Badge Board Support Package, which is configurable from `menuconfig`.

**Supported hardware:**

|   Version   | Status/Supported |
|-------------|------------------|
| 0.8.15      |:heavy_check_mark:|

> Currently only the 0.8.15 was tested. Different versions may work with a different configuration.

**Supported features:**

|   Feature   | Status/Supported |
|-------------|------------------|
| LED         |:heavy_check_mark:|
| RGB LED     |:heavy_check_mark:|
| Vibration   |:heavy_check_mark:|
| Fuel Gauge  |:heavy_check_mark:|
| Buttons     |:heavy_check_mark:|
| IrDA        |       :x:        |
| VOC sensor  |       :x:        |
| NFC         |       :x:        |
| Sec Element |       :x:        |
| Display     |       :x:        |

## Dependencies

|  Component  |                                                     Link                                                     |
|-------------|--------------------------------------------------------------------------------------------------------------|
| button      |[espressif/button](https://components.espressif.com/components/espressif/button)                              |
| led_strip   |[espressif/led_strip](https://components.espressif.com/components/espressif/led_strip)                        |
| max17048    |[espressif/max17048](https://components.espressif.com/components/espressif/max17048)                          |
| i2c_bus     |[espressif/i2c_bus](https://components.espressif.com/components/espressif/i2c_bus)                            |
| ir_learn    |[espressif/ir_learn](https://components.espressif.com/components/espressif/ir_learn)                          |
| vibramotor  |[hope-badge/vibramotor](https://components.espressif.com/components/hope-badge/vibramotor)                                                                |

## How to use

```bash
idf.py add-dependency "hope-badge/hope-badge^0.0.1"
```

## API Reference

### I2C Bus

```c
esp_err_t bsp_i2c_init(void);
esp_err_t bsp_i2c_deinit(void);
```

### Buttons

```c
esp_err_t bsp_buttons_init(void);
button_handle_t bsp_get_button_handle(uint8_t btn_num);
```

### LED (Single IO LED)

```c
esp_err_t bsp_led_init(void);
esp_err_t bsp_gpio_set_level(gpio_num_t gpio_num, uint32_t level);
int8_t bsp_gpio_get_level(gpio_num_t gpio_num);
```

### RGB LED Strip (WS2812)

```c
esp_err_t bsp_init_led_rgb(void);
led_strip_handle_t bsp_get_led_rgb_handle(void);
```

### Battery Fuel Gauge (MAX17048)

```c
esp_err_t bsp_fuel_gauge_init(void);
float bsp_get_battery_voltage(void);
float bsp_get_battery_percentage(void);
```

### BSP Initialization

```c
esp_err_t bsp_init(void);
```

Calls all the necessary `bsp_*_init()` functions:

- I2C
- Buttons
- IO LED
- RGB LED
- Fuel gauge

---

## Example Usage

To use this BSP, please take a look at the example [basic](/examples/basic/README.md).

### Initialize the BSP

```c
ESP_ERROR_CHECK(bsp_init());
```

### Read Battery Voltage and Percentage

```c
float voltage = bsp_get_battery_voltage();
float percentage = bsp_get_battery_percentage();
```

### Control IO LED

```c
bsp_gpio_set_level(BSP_LED_IO, 1); // Turn LED ON
bsp_gpio_set_level(BSP_LED_IO, 0); // Turn LED OFF
```

### Control RGB LED

```c
led_strip_handle_t led_rgb = bsp_get_led_rgb_handle();
led_strip_set_pixel(led_rgb, 0, 10, 0, 0); // Set first pixel to red
led_strip_refresh(led_rgb);
```

### Button Callback Registration (Example)

```c
button_handle_t btn1 = bsp_get_button_handle(BSP_BUTTON_1_GPIO_INDEX);
iot_button_register_cb(btn1, BUTTON_PRESS_DOWN, NULL, my_button_callback, NULL);
```

---

## Notes

- `bsp_init()` will initialize the entire board peripherals and is recommended as the first call in `app_main()`.
- Each `bsp_*_init()` function can be called separately if needed.
- `bsp_i2c_deinit()` can be used to safely shutdown the I2C bus.
- RGB LED control uses RMT backend for accurate timing.
- Fuel gauge APIs return `-1.0` in case of error or if not initialized.

## Resources

[Wiki HOPE XV Electronic Badge](https://wiki.hope.net/index.php?title=HOPE_XV_Electronic_Badge) 
[Hacking in Parallel Badge GitLab Repo](https://gitlab.com/tidklaas/hip-badge)

## License

Please refer to the [LICENSE](LICENSE) file.
