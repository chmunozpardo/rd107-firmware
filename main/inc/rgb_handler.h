#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"

// WS2812 Low level and High level definition
#define WS2812_ON       0xF8
#define WS2812_OFF      0xE0

// SPI configuration for WS2812
#define PIN_NUM_MISO    -1
#define PIN_NUM_MOSI    23
#define PIN_NUM_CLK     -1
#define PIN_NUM_CS      -1

// WS2812 parameters
#define RGB_LEDS        14  // Number of pixels
#define RGB_DATA_N      24  // Total bits per pixel
#define RGB_RESET_TIME  40  // Reset time

// Simple color definitions
#define RGB_RED     255,   0,   0
#define RGB_GREEN     0, 255,   0
#define RGB_BLUE      0,   0, 255
#define RGB_CYAN      0, 181, 181
#define RGB_MAGENTA 181,   0, 181
#define RGB_YELLOW  181, 181,   0
#define RGB_WHITE   147, 147, 147

#define RGB_IDLE      RGB_CYAN

void rgb_init(void);
void rgb_rainbow_leds(void);
void rgb_fixed_leds(uint8_t r, uint8_t g, uint8_t b);


#ifdef __cplusplus
}
#endif
