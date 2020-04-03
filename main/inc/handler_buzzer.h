#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

void buzzer_init(void);
void buzzer_task(void *arg);

#ifdef __cplusplus
}
#endif
