# Vibramotor Component for ESP-IDF

[![Component Registry](https://components.espressif.com/components/hope-badge/vibramotor/badge.svg)](https://components.espressif.com/components/hope-badge/vibramotor)

## Overview

**Vibramotor** is an ESP-IDF component that provides a simple API to control a vibration motor connected to a GPIO pin.

## Features

- Simple GPIO-based control of a vibration motor
- Run vibration cycles asynchronously (non-blocking main app)
- Adjustable ON time, OFF time, and number of cycles
- Ability to stop the motor early
- Uses FreeRTOS task internally

## Hardware Requirements

- ESP32 or compatible SoC running ESP-IDF
- DC vibration motor (either driven directly from GPIO or via transistor/MOSFET)
- GPIO pin connected to the motor driver circuit

## Installation

Install the component via the component manager from the ESP-IDF or search the component using the VSCode Extension for the ESP-IDF.

## Usage

```bash
idf.py add-dependency "hope-badge/vibramotor^0.0.1"
```

### 1. Include the header

```c
#include "vibramotor.h"
```

### 2. Initialize the vibration motor

```c
ESP_ERROR_CHECK(vibramotor_init(GPIO_NUM_5)); // Replace with your GPIO pin
```

### 3. Run the motor with a vibration pattern

```c
// Run motor with:
// ON for 200 ms
// OFF for 300 ms
// Repeat for 5 cycles
ESP_ERROR_CHECK(vibramotor_run(200, 300, 5));
```

### 4. Stop the motor

```c
vibramotor_stop();
```

## API Reference

### `esp_err_t vibramotor_init(uint8_t gpio_num);`

Initializes the vibration motor by configuring the specified GPIO pin as an output.

- **gpio_num**: GPIO number where the motor is connected  
- Returns: `ESP_OK` on success, error code otherwise.

### `esp_err_t vibramotor_run(uint16_t time_on_ms, uint16_t time_off_ms, uint16_t cycles);`

Starts a background task to run the vibration motor in a pulsed pattern.

- **time_on_ms**: Duration to turn ON the motor (milliseconds)  
- **time_off_ms**: Duration to turn OFF the motor (milliseconds)  
- **cycles**: Number of ON/OFF cycles to perform  
- Returns: `ESP_OK` on success, error code otherwise.

> Note: The task runs asynchronously; the main app continues running.

### `void vibramotor_stop(void);`

Stops the current vibration task if it is running.  
The motor will be turned OFF and the task will be deleted.

## Example

```c
#include "vibramotor.h"

void app_main(void)
{
    // Initialize motor on GPIO 5
    ESP_ERROR_CHECK(vibramotor_init(GPIO_NUM_5));

    // Vibrate 10 times: 100ms ON, 100ms OFF
    ESP_ERROR_CHECK(vibramotor_run(100, 100, 10));

    // Optional: Stop the motor early
    // vTaskDelay(500 / portTICK_PERIOD_MS);
    // vibramotor_stop();
}
```

## Notes

- The motor can be driven directly from the GPIO pin only if it requires low current (check your datasheet!).  
  Otherwise, use an NPN transistor, MOSFET, or motor driver circuit.
- Add a flyback diode if using an inductive motor.
- This component uses a FreeRTOS task to manage vibration timing.

## License

See the [license](LICENSE) file.
