#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/gpio.h"

#define BUZZER_GPIO   21

void buzzer_init(void);

#ifdef __cplusplus
}
#endif
