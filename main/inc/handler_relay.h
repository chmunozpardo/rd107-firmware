#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

void relay_init(void);
void relay_task(void *arg);

#ifdef __cplusplus
}
#endif
