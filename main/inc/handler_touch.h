#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

void TP_Dialog(void);
void TP_DrawBoard(void);
void touch_init(void);
void debounce_task(void *arg);

#ifdef __cplusplus
extern "C" {
#endif