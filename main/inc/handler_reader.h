#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

void reader_init(void);
void reader_task(void *arg);

#ifdef __cplusplus
}
#endif
