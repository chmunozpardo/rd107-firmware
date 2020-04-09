#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

void search_card(uint8_t size, uint64_t value);
void search_reservation_qr(char *qr);
void search_reservation_code(char *code);

#ifdef __cplusplus
}
#endif
