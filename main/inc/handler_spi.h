#pragma once

#include "definitions.h"

void spi_init();
void rgb_spi_transmit();
void rgb_spi_delay();
void screen_write_byte(uint8_t Data);
void screen_write_word(uint16_t Data, uint32_t DataLen);
void SPI4W_Write_Byte(uint8_t Data);
uint16_t SPI4W_Read_Byte(uint8_t Data);