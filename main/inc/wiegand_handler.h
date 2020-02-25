#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"

#define WIEGAND_D0    14
#define WIEGAND_D1    27

#define ESP_INTR_FLAG_DEFAULT 0

void wiegand_init(void);
void wiegand_read(void *arg);

#ifdef __cplusplus
}
#endif
