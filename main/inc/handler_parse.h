#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

void parse_qr(void);
void parse_data(void);
void parse_command(void);
void parse_register(void);

#ifdef __cplusplus
}
#endif
