#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rgb_handler.h"

static const char* TAG = "rgb_handler";

static esp_err_t ret;
static spi_device_handle_t spi;
static spi_transaction_t t;
static spi_transaction_t t_emp;

static uint8_t data = 0;
static uint8_t array[24] = {0};
static uint8_t r[RGB_LEDS] = {255, 243, 181,  80,   0,   0,   0,   0,   0,   0,   0,  80, 181, 243};
static uint8_t g[RGB_LEDS] = {  0,  80, 181, 243, 243, 181,  80,   0,  80, 181, 243, 243, 181,  80};
static uint8_t b[RGB_LEDS] = {  0,   0,   0,   0,  80, 181, 243, 255, 243, 181,  80,   0,   0,   0};

static void rgb_ws2812(uint8_t r, uint8_t g, uint8_t b){
    for(int i = 0; i < 8; i++)
        array[i] = ((g >> (7 - i)) & (0x01))== 0x01 ? WS2812_ON : WS2812_OFF;
    for(int i = 0; i < 8; i++)
        array[i+8] = ((r >> (7 - i)) & (0x01)) == 0x01 ? WS2812_ON : WS2812_OFF;
    for(int i = 0; i < 8; i++)
        array[i+16] = ((b >> (7 - i)) & (0x01)) == 0x01 ? WS2812_ON : WS2812_OFF;
}

void rgb_init(void){
    memset(&t, 0, sizeof(t));
    t.length = 8*RGB_DATA_N;
    t.tx_buffer = &array;
    t.user = (void*)0;

    memset(&t_emp, 0, sizeof(t_emp));
    t_emp.length = 8;
    t_emp.tx_buffer = &data;
    t_emp.user = (void*)0;

    spi_bus_config_t buscfg={
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = RGB_DATA_N*RGB_LEDS
    };

    spi_device_interface_config_t devcfg={
        .clock_speed_hz = 6400000,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 40,
    };

    ESP_LOGI(TAG, "Initializing SPI for RGB");
    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}

void rgb_rainbow_leds(void){
    for(int j = 0; j < RGB_LEDS; j++){
        rgb_ws2812(r[j%RGB_LEDS], g[j%RGB_LEDS], b[j%RGB_LEDS]);
        ret = spi_device_polling_transmit(spi, &t);
    }
    for(int j = 0; j < RGB_RESET_TIME; j++){
        ret = spi_device_polling_transmit(spi, &t_emp);
    }
}

void rgb_fixed_leds(uint8_t r, uint8_t g, uint8_t b){
    for(int j = 0; j < RGB_LEDS; j++){
        rgb_ws2812(r, g, b);
        ret = spi_device_polling_transmit(spi, &t);
    }
    for(int j = 0; j < RGB_RESET_TIME; j++){
        ret = spi_device_polling_transmit(spi, &t_emp);
    }
}