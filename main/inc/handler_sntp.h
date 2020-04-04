#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

void ntp_task(void *arg);
void ntp_init(void);

#ifdef __cplusplus
}
#endif
