#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct __attribute__((packed, aligned(1))) card_structure{
    uint8_t cardType;
    uint8_t permisos;
    uint8_t antipassbackStatus;
    uint16_t canalHorario;
    uint32_t code1;
    uint32_t code2;
} card_structure;

typedef struct __attribute__((packed, aligned(1))) simple_structure{
    uint8_t cardType;
    uint32_t code1;
    uint32_t code2;
} simple_structure;

#define CARD_FULL_SIZE      13
#define CARD_SIMPLE_SIZE     9

void card_search(uint8_t size, uint64_t value);
void card_action(bool arg);

#define CARD_COMPARE(A, B) A.cardType == B.cardType && A.code1 == B.code1 && A.code2 == B.code2
#define MIFARE(value) (value & 0x000000FF) << 24 | \
                      (value & 0x0000FF00) <<  8 | \
                      (value & 0x00FF0000) >>  8 | \
                      (value & 0xFF000000) >> 24

#define HID_CODE1(value) (value >> (17 + 6)) & 0x00FF
#define HID_CODE2(value) (value >> ( 1 + 6)) & 0xFFFF

#ifdef __cplusplus
}
#endif
