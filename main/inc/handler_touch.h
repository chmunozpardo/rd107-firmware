#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

void touch_init_f(void);
void touch_input_keyboard(void);
void debounce_task(void *arg);

#ifdef __cplusplus
extern "C" {
#endif