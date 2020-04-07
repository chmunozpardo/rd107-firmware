#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

void data_register(char *code, uint8_t *mac, uint32_t ip, uint32_t gw);
void data_task(void *arg);

#ifdef __cplusplus
}
#endif
