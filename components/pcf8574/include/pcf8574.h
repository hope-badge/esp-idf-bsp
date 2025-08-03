/**
 * @file
 * @brief ESP BSP: Generic
 */

#pragma once

#include "sdkconfig.h"
#include "i2c_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PCF8574_I2C_ADDR_DEFAULT 0x20

// PCF8574 I2C Registers
#define PCF8574_REG_INPUT 0x00
#define PCF8574_REG_OUTPUT 0x01
#define PCF8574_REG_POLARITY 0x02
#define PCF8574_REG_CONFIG 0x03

typedef void *pcf8574_handle_t;


#ifdef __cplusplus
}
#endif
