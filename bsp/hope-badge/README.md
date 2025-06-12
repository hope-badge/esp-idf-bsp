# HOPE Badge Board Support Package (BSP)

### Overview

This is the HOPE Badge Board Support Package, which is configurable from `menuconfig`.

**Supported features:**

|   Feature   | Status/Supported |
|-------------|------------------|
| LED         |:heavy_check_mark:|
| RGB LED     |:heavy_check_mark:|
| Vibration   |:heavy_check_mark:|
| Fuel Gauge  |:heavy_check_mark:|
| Buttons     |:heavy_check_mark:|
| Temp Sensor |       :x:        |
| NFC         |       :x:        |
| Sec Element |       :x:        |
| Display     |       :x:        |

### Dependencies

|  Capability |                                                   Component                                                  |
|-------------|--------------------------------------------------------------------------------------------------------------|
| button      |[espressif/button](https://components.espressif.com/components/espressif/button)                              |
| led_strip   |[espressif/led_strip](https://components.espressif.com/components/espressif/led_strip)                           |
| max17048    |[espressif/max17048](https://components.espressif.com/components/espressif/max17048)                            |
| i2c_bus     |[espressif/i2c_bus](https://components.espressif.com/components/espressif/i2c_bus)                             |
| ir_learn    |[espressif/ir_learn](https://components.espressif.com/components/espressif/ir_learn)                            |
| vibrator    |                                                                                                              |

### License

Please refer to the [LICENSE](LICENSE) file.
