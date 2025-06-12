# HOPE Badge Board Support Package (BSP)

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
| vibramotor  |                                                                                                              |

## License

Please refer to the [LICENSE](LICENSE) file.
