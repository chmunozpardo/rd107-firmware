#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/spi_master.h"

#include "esp_http_server.h"
#include "esp_http_client.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_attr.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_task_wdt.h"
#include "esp_int_wdt.h"

#include "lwip/inet.h"
#include "lwip/ip4_addr.h"
#include "lwip/dns.h"

#include "soc/uart_struct.h"

#include "nvs_flash.h"
#include "tcpip_adapter.h"

#include "cJSON.h"

#define RD_MODELO           "RD-107"
#define RD_VERSION          "0.1"
#define RD_SERIE            "cmunoz"
#define RD_CANALES          "1"

#define LOCAL_TIMEZONE      "<-04>4"

#define URL                 "http://192.168.1.88:8080/control_acceso/obtenerMediosAccesoControladorBinario"
#define URL_COMMAND         "http://192.168.1.88:8080/control_acceso/obtenerComandosManualesPendientesControlador"
#define URL_QR              "http://192.168.1.88:8080/control_acceso/obtenerCodigoQR"
#define URL_RESERVATIONS    "http://192.168.1.88:8080/control_acceso/obtenerReservasBinario"
#define URL_REG             "http://192.168.1.88:8080/control_acceso/registrarControlador"

#define HTTPS_BUFFER        (4096+0)

#define FILE_JSON           "/spiffs/registers.json"
#define FILE_CARDS          "/spiffs/registers.db"
#define FILE_RESERVATIONS   "/spiffs/reservations.db"
#define FILE_CONFIG         "/spiffs/config.txt"
#define FILE_WIFI           "/spiffs/wifi.txt"
#define FILE_TIMESTAMP      "/spiffs/timestamp.txt"

#define COPY_SIZE           512

#define BUZZER_GPIO         21
#define RELAY_GPIO          22

typedef struct __attribute__((packed, aligned(1))) card_structure{
    uint8_t cardType;
    uint8_t permisos;
    uint8_t antipassbackStatus;
    uint16_t canalHorario;
    uint32_t code1;
    uint32_t code2;
    uint32_t index;
} card_structure;

typedef struct __attribute__((packed, aligned(1))) reservation_structure{
    char qr[8];
    char code[6];
    uint64_t init_time;
    uint64_t end_time;
    uint32_t index;
} reservation_structure;

typedef union ip4_str
{
    ip4_addr_t ip_addr_i;
    struct
    {
        uint8_t addr[4];
    };
} ip4_str;

#define CARD                    card_structure
#define RESERVATION             reservation_structure

#define CARD_SIZE               sizeof(CARD)
#define CARD_READER_SIZE        (2048)
#define RESERVATION_SIZE        sizeof(RESERVATION)
#define RESERVATION_READER_SIZE (1024)

#define CARD_COMPARE(A, B)              A.cardType == B.cardType && A.code1 == B.code1 && A.code2 == B.code2
#define RESERVATION_COMPARE_QR(A, B)    strncmp(A, B.qr, 6);
#define RESERVATION_COMPARE_CODE(A, B)  strncmp(A, B.code, 6);

#define MIFARE(value)       (value & 0x000000FF) << 24 | \
                            (value & 0x0000FF00) <<  8 | \
                            (value & 0x00FF0000) >>  8 | \
                            (value & 0xFF000000) >> 24

#define HID_CODE1(value)    (value >> (17 + 6)) & 0x00FF
#define HID_CODE2(value)    (value >> ( 1 + 6)) & 0xFFFF

// WS2812 Low level and High level definition
#define WS2812_ON       0xF8
#define WS2812_OFF      0xE0

// SPI configuration for WS2812
#define PIN_NUM_MISO    -1
#define PIN_NUM_MOSI    4
#define PIN_NUM_CLK     -1
#define PIN_NUM_CS      -1

// SPI configuration for TFT 3.5" display
#define LCD_FREQ            20000000

#define LCD_PIN_CLK         25
#define LCD_PIN_MISO        27
#define LCD_PIN_MOSI        33
#define LCD_PIN_CS          5

#define LCD_PIN_DC          23
#define LCD_PIN_IRQ         26

#define QR_SIZE             14
#define QR_OFFSET           (320 - QR_SIZE * 21)/2
#define SCREEN_BUFFER       1024

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

// Screen default values
#define LCD_BACKGROUND  WHITE //Default background color
#define FONT_BACKGROUND WHITE //Default font background color
#define FONT_FOREGROUND GRED  //Default font foreground color

// Screen colors
#define WHITE   0xFFFF
#define BLACK   0x0000
#define BLUE    0x001F
#define BRED    0XF81F
#define GRED    0XFFE0
#define GBLUE   0X07FF
#define RED     0xF800
#define MAGENTA 0xF81F
#define GREEN   0x07E0
#define CYAN    0x7FFF
#define YELLOW  0xFFE0
#define BROWN   0XBC40
#define BRRED   0XFC07
#define GRAY    0X8430

#define RGB_IDLE      RGB_CYAN

#define WIEGAND_D0    14
#define WIEGAND_D1    17

#define ESP_INTR_FLAG_DEFAULT          0
#define CONFIG_ESP_TASK_WDT_TIMEOUT_MS 5
#define DEFAULT_SCAN_LIST_SIZE         20

extern char apitoken[30];
extern char database[20];
extern char idcontrolador[3];

extern char strftime_buf[32];
extern char strftime_buf_end[32];

extern ip4_str ip_addr;
extern ip4_str gw_addr;
extern uint8_t mac[6];

extern time_t system_now;

extern uint8_t  loaded_data;
extern uint32_t card_size;
extern uint32_t reservation_size;
extern uint64_t timestamp;
extern uint64_t timestamp_temp;

extern CARD card_importer[COPY_SIZE];
extern CARD card_data[CARD_READER_SIZE];

extern RESERVATION reservation_importer[COPY_SIZE];
extern RESERVATION reservation_data[RESERVATION_READER_SIZE];

extern xQueueHandle qr_task_queue;
extern xQueueHandle ntp_task_queue;
extern xQueueHandle rgb_task_queue;
extern xQueueHandle relay_task_queue;
extern xQueueHandle buzzer_task_queue;

extern SemaphoreHandle_t reg_semaphore;
extern SemaphoreHandle_t reservation_semaphore;

extern TaskHandle_t qr_task_handle;
extern TaskHandle_t ntp_task_handle;
extern TaskHandle_t rgb_task_handle;
extern TaskHandle_t data_task_handle;
extern TaskHandle_t relay_task_handle;
extern TaskHandle_t buzzer_task_handle;
extern TaskHandle_t wiegand_task_handle;

extern char screen_qr[6];

#define RGB_SIGNAL(rgb_value, rgb_leds, rgb_s) {\
                                        uint8_t rgb[5] = {rgb_value, rgb_leds, rgb_s};\
                                        xQueueSend(rgb_task_queue, &rgb, (unsigned int) 0);\
                                        }

#define RELAY_SIGNAL(relay_s) {\
                            uint8_t relay = relay_s;\
                            xQueueSend(relay_task_queue, &relay, (unsigned int) 0);\
                            }

#define BUZZER_SIGNAL(buzzer_s) {\
                            uint8_t buzzer = buzzer_s;\
                            xQueueSend(buzzer_task_queue, &buzzer, (unsigned int) 0);\
                            }

#define QR_SIGNAL() {\
                    xQueueSend(qr_task_queue, &screen_qr, (unsigned int) 0);\
                    }
