/**
 * @file
 * @brief ESP BSP: Generic
 */

#pragma once

#include "sdkconfig.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t time_on_ms;
    uint32_t time_off_ms;
    uint32_t repeat_count;
} vibramotor_params_t;

esp_err_t vibramotor_init(uint8_t gpio_num);
esp_err_t vibramotor_run(uint16_t time_on_ms, uint16_t time_off_ms, uint16_t cycles);
esp_err_t vibramotor_stop(void);

#ifdef __cplusplus
}
#endif
