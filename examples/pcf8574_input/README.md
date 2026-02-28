# PCF8574 Input Reading Example

This example demonstrates how to use the **PCF8574** I²C I/O expander component to read input pins, using both **polling** and **interrupt-driven** approaches.

## Overview

The PCF8574 has 8 quasi-bidirectional I/O pins. Pins configured as inputs are held HIGH by a weak internal pull-up (~100 µA). When a button or switch pulls a pin LOW, the change is detected either by periodic polling or via the PCF8574 INT output.

This example configures:
- **P1, P2, P3** as inputs (buttons to GND)
- **P0, P4–P7** as outputs
- P0 is set HIGH as a demo

## Hardware Setup

```
                 ┌─────────────┐
           P0 ── │ PCF8574     │ ── VCC (3.3V)
   BTN1 ── P1 ── │             │ ── GND
   BTN2 ── P2 ── │  I2C Addr:  │ ── SDA (GPIO 20)
   BTN3 ── P3 ── │  0x20       │ ── SCL (GPIO 21)
           P4 ── │             │ ── INT (optional → GPIO 4)
           P5 ── │             │
           P6 ── │             │
           P7 ── │             │
                 └─────────────┘
```

- **Buttons**: Connect between PCF8574 input pins (P1, P2, P3) and **GND**. No external pull-up is needed — the PCF8574 has a weak internal pull-up.
- **INT pin** (optional): Connect to a host GPIO (e.g., GPIO 4) with an external pull-up resistor (4.7 kΩ to VCC), since INT is open-drain.

## Two Approaches

### 1. Polling (default)

Reads all PCF8574 pins every 500 ms and logs changes:

```
I (1234) pcf8574_input: Port read: 0xFC (bin: 11111100)
I (1234) pcf8574_input:   P1 = 0 (pressed)
I (1234) pcf8574_input:   P2 = 0 (pressed)
I (1234) pcf8574_input:   P3 = 1 (released)
```

### 2. Interrupt-driven

Reacts immediately when any input pin changes. To enable this mode, uncomment `#define EXAMPLE_USE_INTERRUPT` in `main.c` and set `PCF8574_INT_GPIO` to the host GPIO connected to the PCF8574 INT pin.

```
I (5678) pcf8574_input: [INT] Port read: 0xFA
I (5678) pcf8574_input: [INT]   P1 = 1 (released)
I (5678) pcf8574_input: [INT]   P2 = 0 (pressed)
I (5678) pcf8574_input: [INT]   P3 = 1 (released)
```

## Build and Flash

```bash
cd examples/pcf8574_input
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Key API Calls Demonstrated

| Function                      | Purpose                                       |
|-------------------------------|-----------------------------------------------|
| `pcf8574_create()`            | Create the device handle                      |
| `pcf8574_set_direction()`     | Set which pins are inputs vs outputs          |
| `pcf8574_read()`              | Read all 8 pins at once                       |
| `pcf8574_read_pin()`          | Read a single pin                             |
| `pcf8574_set_pin()`           | Set an individual output pin HIGH             |
| `pcf8574_register_interrupt()`| Register an ISR for pin-change events         |

## License

This example is in the Public Domain (or CC0 licensed, at your option).
