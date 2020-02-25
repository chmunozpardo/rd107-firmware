#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct card_structure{
    uint8_t cardType;
    uint32_t code1;
    uint32_t code2;
    uint8_t permisos;
    uint16_t canalHorario;
    uint8_t antipassbackStatus;
} card_structure;

typedef struct simple_structure{
    uint8_t cardType;
    uint32_t code1;
    uint32_t code2;
} simple_structure;

bool card_compare(const simple_structure a, const simple_structure b);
void card_search(uint8_t size, uint64_t value);
void card_action(bool arg);

#ifdef __cplusplus
}
#endif
