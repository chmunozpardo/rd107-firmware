#pragma once

#include "definitions.h"

void spi_init();
void rgb_spi_transmit();
void rgb_spi_delay();
void SPI4W_Write_Byte(uint8_t Data);
uint16_t SPI4W_Read_Byte(uint8_t Data);