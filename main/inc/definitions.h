#pragma once

//#define DEBUG_INFO

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_http_client.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"

#include "nvs_flash.h"
#include "tcpip_adapter.h"

typedef struct __attribute__((packed, aligned(1))) card_structure{
    uint8_t cardType;
    uint8_t permisos;
    uint8_t antipassbackStatus;
    uint16_t canalHorario;
    uint32_t code1;
    uint32_t code2;
    uint32_t index;
} card_structure;

#define URL              "http://192.168.10.241/control_acceso/obtenerMediosAccesoControladorBinario"
#define ID_CONTROLADOR   "16"
#define DATABASE         "GK2_Titanium"
#define API_TOKEN        "dreamit-testing-rd107-2020"
#define NOMBRE_INSTANCIA "GK2_Titanium"

#define REG_FILE_JSON   "/spiffs/registers.json"
#define REG_FILE        "/spiffs/registers.db"
#define REG_TIMESTAMP   "/spiffs/timestamp.db"

#define COPY_SIZE           1024

#define BUZZER_GPIO         21

#define CARD_FULL_SIZE      17
#define CARD_READER_SIZE    8000
#define CARD                card_structure

#define CARD_COMPARE(A, B) A.cardType == B.cardType && A.code1 == B.code1 && A.code2 == B.code2
#define MIFARE(value) (value & 0x000000FF) << 24 | \
                      (value & 0x0000FF00) <<  8 | \
                      (value & 0x00FF0000) >>  8 | \
                      (value & 0xFF000000) >> 24

#define HID_CODE1(value) (value >> (17 + 6)) & 0x00FF
#define HID_CODE2(value) (value >> ( 1 + 6)) & 0xFFFF

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

#define WIEGAND_D0    14
#define WIEGAND_D1    27

#define ESP_INTR_FLAG_DEFAULT 0

extern uint32_t registers_size;
extern uint64_t timestamp;
extern CARD *registers_data;
extern xQueueHandle rgb_task_queue;
extern SemaphoreHandle_t reg_semaphore;
extern SemaphoreHandle_t rgb_semaphore;

#define RGB_SIGNAL(rgb_value, rgb_leds, rgb_ms) {\
                                        xSemaphoreTake(rgb_semaphore, portMAX_DELAY);\
                                        uint8_t rgb[4] = {rgb_value, rgb_leds};\
                                        xQueueSend(rgb_task_queue, &rgb, (unsigned int) 0);\
                                        if(rgb_ms > 0) vTaskDelay(rgb_ms/portTICK_PERIOD_MS);\
                                        xSemaphoreGive(rgb_semaphore);\
                                        }
