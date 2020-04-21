#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

void spi_init();
void rgb_spi_transmit();
void rgb_spi_delay();
void screen_write_byte(uint8_t Data);
void screen_write_word(uint16_t Data, uint32_t DataLen);
void screen_write_buffer(uint32_t DataLen);
uint16_t screen_read_byte(uint8_t Data);

#ifdef __cplusplus
}
#endif
