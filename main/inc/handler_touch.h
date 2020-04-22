#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

void touch_init_f(void);
void touch_input_keyboard(void);
void touch_set_context(void *context, uint8_t context_status);
void debounce_task(void *arg);

#ifdef __cplusplus
extern "C" {
#endif