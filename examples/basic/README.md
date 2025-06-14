# HOPE Badge Basic Example

This is a basic example that demonstrate all the supported features of the HOPE badge BSP.

## Overview

This example demonstrates the basic usage of the **HOPE Badge BSP** along with:

- Button handling
- LED RGB animations
- Simple LED blinking
- Battery monitoring
- Vibramotor control (GPIO-driven)

It shows how to integrate the Vibramotor component with the badge’s BSP features in an event-driven and multitasking environment using FreeRTOS.

## Features

✅ Button callbacks with different events  
✅ LED RGB blinking and ring animation  
✅ LED IO blinking  
✅ Battery voltage and percentage monitoring  
✅ Vibration motor control with adjustable pulse pattern  

## Dependencies

|  Component           |                                                     Link                                                    |
|----------------------|-------------------------------------------------------------------------------------------------------------|
| hope-badge           |[espressif/button](https://components.espressif.com/components/espressif/button)                             |
| network_provisioning |[espressif/network_provisioning](https://components.espressif.com/components/espressif/network_provisioning) |

## Hardware Requirements

- HOPE Badge hardware with supported ESP32 chipset
- Vibramotor connected to `BSP_VIBRAMOTOR_IO` GPIO (via transistor/MOSFET recommended)
- Battery connected for voltage monitoring
- Built-in RGB LED strip
- Built-in buttons

## Example Behavior

### Buttons

- **Button 1 PRESS_DOWN** → Toggle RGB LED blink task
- **Button 2 DOUBLE_CLICK** → Logs event (can be extended)
- **Button 2 LONG_PRESS_START (5 sec)** → Logs event (can be extended)

### Vibramotor

- Runs at startup:
  - 250 ms ON
  - 100 ms OFF
  - 6 cycles

### LED Control

- **RGB Blink Task** → Toggles all RGB LEDs ON/OFF
- **RGB Ring Task** → Runs a rotating LED ring animation (commented out by default)
- **LED Blink Task** → Toggles LED IO pin at 100 ms intervals

### Battery Monitor

- Logs battery voltage and charge percentage every 10 seconds.


## Example Setup

### 1. Initialize BSP

```c
esp_err_t ret = bsp_init();
```

### 2. Register Button Callbacks

```c
btn_register_callbacks();
```

### 3. Initialize Vibramotor

```c
vibramotor_init(BSP_VIBRAMOTOR_IO);
```

### 4. Start Vibramotor

```c
vibramotor_run(250, 100, 6);
```

### 5. Start LED and Battery Tasks

```c
xTaskCreate(&led_blink_task, "led_blink_task", 2048, NULL, 7, NULL);
xTaskCreate(&led_battery_monitor_task, "led_battery_monitor_task", 2048, NULL, 5, NULL);
```

### Optional: Start RGB LED Animations

```c
// xTaskCreate(&led_rgb_blink_task, "led_rgb_blink_task", 2048, NULL, 6, NULL);
// xTaskCreate(&led_rgb_ring_task, "led_rgb_ring_task", 2048, NULL, 8, NULL);
```

## Build and Flash

```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Example Log Output

```
I (00001) badge main: Starting HOPE badge basic example
I (00005) badge main: Hope Badge example initialized successfully
I (01010) badge main: Battery Voltage: 3.85 V, Percentage: 78.5%
```

## Notes

- Vibramotor control is fully asynchronous using a FreeRTOS task.
- Button callbacks are customizable — you can add different behaviors to different button events.
- Use external driver circuitry when connecting a vibration motor.
- The battery monitor relies on BSP APIs to obtain voltage and state of charge.
