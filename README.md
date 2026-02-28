# HOPE Badge BSP

Board Support Package for the [HOPE XV Electronic Badge](https://wiki.hope.net/index.php?title=HOPE_XV_Electronic_Badge), based on the ESP32-C3.

> **ESP-IDF ≥ 5.4 required**

## Project Structure

```
esp-idf-bsp/
├── bsp/hope-badge/          BSP component
├── components/
│   ├── pcf8574/              PCF8574 I²C I/O expander driver
│   └── vibramotor/           Vibration motor driver
├── examples/
│   ├── basic/                Buttons, LEDs, battery monitor, vibramotor demo
│   └── pcf8574_input/        PCF8574 polling & interrupt demo
└── docs/
```

## BSP

| Version | HW Revision | Status |
|---------|-------------|--------|
| 0.0.3   | 0.8.15      | ✅ Tested |
| 0.0.3   | 0.8.16      | ⚠️ Untested |
| 0.0.3   | 0.8.17      | ⚠️ Untested |

### Supported Features

| Feature         | Status | Notes |
|-----------------|--------|-------|
| Buttons (1–4)   | ✅     | Buttons 3 & 4 share USB pins — enable via code |
| LED             | ✅     | Single GPIO LED |
| RGB LED (WS2812)| ✅     | 16-pixel ring via RMT |
| Vibration Motor | ✅     | Async FreeRTOS-based control |
| Fuel Gauge      | ✅     | MAX17048 — voltage & percentage |
| I/O Expander    | ✅     | PCF8574 / PCF8574A with auto-detect |
| IrDA            | ❌     | Pins defined, not yet implemented |
| VOC Sensor      | ❌     | Not yet implemented |
| NFC             | ❌     | Not yet implemented |
| Secure Element  | ❌     | Not yet implemented |
| Display         | ❌     | Not yet implemented |

### Default Pin Configuration (v0.8.15)

| Peripheral       | GPIO | Kconfig Key |
|------------------|------|-------------|
| I2C SCL          | 21   | `BSP_I2C_GPIO_SCL` |
| I2C SDA          | 20   | `BSP_I2C_GPIO_SDA` |
| Button 1         | 10   | `BSP_BUTTON_1_GPIO` |
| Button 2         | 9    | `BSP_BUTTON_2_GPIO` |
| Button 3 (USB)   | 19   | `BSP_BUTTON_3_GPIO` |
| Button 4 (USB)   | 18   | `BSP_BUTTON_4_GPIO` |
| LED              | 13   | `BSP_LED_GPIO` |
| RGB LED          | 8    | `BSP_LED_RGB_GPIO` |
| IrDA TX          | 2    | `BSP_IRDA_TX_GPIO` |
| IrDA RX          | 3    | `BSP_IRDA_RX_GPIO` |
| Vibration Motor  | 12   | `BSP_VIBRATION_MOTOR_GPIO` |

All pins are configurable via `idf.py menuconfig` → **HOPE Badge BSP Configuration**.

## Components

| Component    | Version | Description |
|-------------|---------|-------------|
| [hope-badge](bsp/hope-badge/) | 0.0.3 | Main BSP — I2C, buttons, LEDs, fuel gauge, I/O expander |
| [pcf8574](components/pcf8574/) | 0.0.1 | PCF8574/PCF8574A 8-bit I²C I/O expander driver |
| [vibramotor](components/vibramotor/) | 0.0.3 | GPIO-based vibration motor with async FreeRTOS control |

### External Dependencies

| Dependency | Version | Source |
|-----------|---------|--------|
| [button](https://components.espressif.com/components/espressif/button) | ^4 | Espressif Component Registry |
| [led_strip](https://components.espressif.com/components/espressif/led_strip) | ^3 | Espressif Component Registry |
| [max17048](https://components.espressif.com/components/espressif/max17048) | ^0.1.1 | Espressif Component Registry |
| [i2c_bus](https://components.espressif.com/components/espressif/i2c_bus) | ^1.1 | Espressif Component Registry |

## Examples

| Example | Description |
|---------|-------------|
| [basic](examples/basic/) | Full demo — button callbacks, RGB LED animations, LED blink, battery monitoring, vibramotor |
| [pcf8574_input](examples/pcf8574_input/) | PCF8574 input reading with polling and interrupt modes |

## Getting Started

### Using as a dependency

Add the BSP to your project's `main/idf_component.yml`:

```yaml
dependencies:
  hope-badge/hope-badge:
    version: "0.0.3"
```

### Creating a project from the example

```bash
idf.py create-project-from-example "hope-badge/hope-badge:basic"
cd basic
idf.py set-target esp32c3
idf.py build
```

### Quick usage

```c
#include "bsp/bsp.h"

void app_main(void)
{
    // Initialize all BSP peripherals (I2C, buttons, LEDs, fuel gauge, PCF8574)
    ESP_ERROR_CHECK(bsp_init());

    // Read battery
    float voltage = bsp_get_battery_voltage();
    float percent = bsp_get_battery_percentage();

    // Control LED
    bsp_gpio_set_level(BSP_LED_IO, 1);

    // Use RGB LED strip
    led_strip_handle_t strip = bsp_get_led_rgb_handle();
    led_strip_set_pixel(strip, 0, 255, 0, 0);
    led_strip_refresh(strip);
}
```

## Resources

- [HOPE XV Badge Wiki](https://wiki.hope.net/index.php?title=HOPE_XV_Electronic_Badge)
- [HIP Badge Hardware (GitLab)](https://gitlab.com/tidklaas/hip-badge)
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/stable/)
- [Espressif Component Registry](https://components.espressif.com/)

## License

This project is in the Public Domain (or CC0 licensed, at your option).
