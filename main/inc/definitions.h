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

#define RD_MODELO               "RD-107"
#define RD_VERSION              "0.1"
#define RD_SERIE                "cmunoz"
#define RD_CANALES              "1"

#define DISPLAY_TIME            2

#define COLOR                   uint16_t
#define POINT                   uint16_t
#define LENGTH                  uint16_t

#define LCD_X_MAXPIXEL          480
#define LCD_Y_MAXPIXEL          320
#define LCD_X                   0
#define LCD_Y                   0

#define LCD_WIDTH               (LCD_X_MAXPIXEL - 2 * LCD_X)
#define LCD_HEIGHT              (LCD_Y_MAXPIXEL)

#define SCAN_DIR_DFT            D2U_L2R
#define DOT_STYLE_DFT           DOT_FILL_AROUND
#define DOT_PIXEL_DFT           DOT_PIXEL_1X1

#define LOCAL_TIMEZONE          "<-04>4"
#define LCD_KEYBOARD_MAY        "1234567890QWERTYUIOPASDFGHJKL<_ZXCVBNM >"
#define LCD_KEYBOARD_MIN        "1234567890qwertyuiopasdfghjkl<^zxcvbnm >"

#define HOSTNAME                "http://192.168.1.88:8080/"
//#define HOSTNAME                "https://alpha-api.gestkontrol.cl/"
#define URL                     HOSTNAME"control_acceso/obtenerMediosAccesoControladorBinario"
#define URL_COMMAND             HOSTNAME"control_acceso/obtenerComandosManualesPendientesControlador"
#define URL_QR                  HOSTNAME"control_acceso/obtenerCodigoQR"
#define URL_VALIDATION          HOSTNAME"control_acceso/obtenerValidacion"
#define URL_RESERVATIONS        HOSTNAME"control_acceso/obtenerReservasBinario"
#define URL_REG                 HOSTNAME"control_acceso/registrarControlador"

#define FILE_JSON               "/spiffs/registers.json"
#define FILE_CARDS              "/spiffs/registers.db"
#define FILE_RESERVATIONS       "/spiffs/reservations.db"
#define FILE_RUTS               "/spiffs/ruts.db"

#define FILE_CONFIG             "/spiffs/config.txt"
#define FILE_WIFI               "/spiffs/wifi.txt"
#define FILE_TIMESTAMP          "/spiffs/timestamp.txt"

#define RUT                     rut_t
#define CARD                    card_t
#define RESERVATION             reservation_t

#define RUT_SIZE                sizeof(RUT)
#define RUT_READER_SIZE         (1024)
#define CARD_SIZE               sizeof(CARD)
#define CARD_READER_SIZE        (2048)
#define RESERVATION_SIZE        sizeof(RESERVATION)
#define RESERVATION_READER_SIZE (1024)

#define HTTPS_BUFFER            (4096)

#define COPY_SIZE               512

#define RUT_COMPARE(A, B)               strncmp(A, B.rut, 10) == 0;
#define CARD_COMPARE(A, B)              A.cardType == B.cardType && A.code1 == B.code1 && A.code2 == B.code2;
#define RESERVATION_COMPARE_QR(A, B)    strncmp(A, B.qr, 8) == 0 && system_now >= B.init_time && system_now <= B.end_time;
#define RESERVATION_COMPARE_RUT(A, B)   A.code1 == B.code1 && A.code2 == B.code2 && system_now >= B.init_time && system_now <= B.end_time;
#define RESERVATION_COMPARE_CODE(A, B)  strncmp(A, B.code, 6) == 0 && system_now >= B.init_time && system_now <= B.end_time;

#define MIFARE(value)       (value & 0x000000FF) << 24 | \
                            (value & 0x0000FF00) <<  8 | \
                            (value & 0x00FF0000) >>  8 | \
                            (value & 0xFF000000) >> 24

#define HID_CODE1(value)    (value >> (17 + 6)) & 0x00FF
#define HID_CODE2(value)    (value >> ( 1 + 6)) & 0xFFFF

// WS2812 Low level and High level definition
#define WS2812_ON           0xF8
#define WS2812_OFF          0xE0

// SPI configuration for WS2812
#define PIN_NUM_MISO        -1
#define PIN_NUM_MOSI        4
#define PIN_NUM_CLK         -1
#define PIN_NUM_CS          -1

#define BUZZER_GPIO         21
#define RELAY_GPIO          22

// SPI configuration for TFT 3.5" display
#define LCD_FREQ            20000000

#define LCD_PIN_CLK         25
#define LCD_PIN_MISO        27
#define LCD_PIN_MOSI        33
#define LCD_PIN_CS          5

#define LCD_PIN_DC          23
#define LCD_PIN_IRQ         26

#define TP_PRESS_DOWN       0x80
#define TP_PRESSED          0x40

#define TOUCH_FREQ          125000
#define TOUCH_PIN_CS        12

#define QR_SIZE             14
#define QR_OFFSET           (320 - QR_SIZE * 21)/2
#define SCREEN_BUFFER       2046

// WS2812 parameters
#define RGB_LEDS            14  // Number of pixels
#define RGB_DATA_N          24  // Total bits per pixel
#define RGB_RESET_TIME      40  // Reset time

// Simple color definitions
#define RGB_RED     255,   0,   0
#define RGB_GREEN     0, 255,   0
#define RGB_BLUE      0,   0, 255
#define RGB_CYAN      0, 181, 181
#define RGB_MAGENTA 181,   0, 181
#define RGB_YELLOW  181, 181,   0
#define RGB_WHITE   147, 147, 147
#define RGB_ORANGE  255, 165,   0
#define RGB_IDLE            RGB_CYAN

// Screen default values
#define LCD_BACKGROUND      LCD_WHITE //Default background color
#define FONT_BACKGROUND     LCD_WHITE //Default font background color
#define FONT_FOREGROUND     LCD_BLACK  //Default font foreground color

// Screen colors
#define LCD_WHITE           0xFFFF
#define LCD_BLACK           0x0000
#define LCD_BLUE            0x001F
#define LCD_GREEN           0x07E0
#define LCD_RED             0xF800
#define LCD_BRED            0XF81F
#define LCD_GRED            0XFFE0
#define LCD_GBLUE           0X07FF
#define LCD_MAGENTA         0xF81F
#define LCD_CYAN            0x7FFF
#define LCD_YELLOW          0xFFE0
#define LCD_BROWN           0XBC40
#define LCD_BRRED           0XFC07
#define LCD_GRAY            0X8430

#define LCD_LOGO_TOP        0x0354
#define LCD_LOGO_BOT        0x3a8c
#define LCD_LOGO_TEXT       0x9515

#define TOUCH_READ_N        4
#define TOUCH_STD_RANGE     30

#define WIEGAND_D0          14
#define WIEGAND_D1          17

#define ESP_INTR_FLAG_DEFAULT           0
#define DEFAULT_SCAN_LIST_SIZE          20
#define CONFIG_ESP_TASK_WDT_TIMEOUT_S   5

typedef struct _tFont
{
    const uint8_t *table;
    uint16_t Width;
    uint16_t Height;
    COLOR color;
} sFONT;

typedef struct screen_queue_t
{
    char msg[7];
    uint8_t status;
    uint8_t timer;
} screen_queue_t;

typedef enum {
    TOUCH_NONE = 0,
    TOUCH_SET_WIFI,
    TOUCH_SET_WIFI_LIST,
    TOUCH_SET_WIFI_PASSWORD,
    TOUCH_SET_DEVICE,
    TOUCH_SET_DEVICE_CODE,
    TOUCH_QR_CODE,
    TOUCH_INPUT_RESERVATION,
    TOUCH_INPUT_RUT,
} TOUCH_CONTEXT;

typedef enum {
    WIFI_WEBSERVER = 0,
    REG_WEBSERVER,
} WEBSERVER_CONTEXT;

typedef enum {
    DOT_PIXEL_1X1 = 1,
    DOT_PIXEL_2X2,
    DOT_PIXEL_3X3,
    DOT_PIXEL_4X4,
    DOT_PIXEL_5X5,
    DOT_PIXEL_6X6,
    DOT_PIXEL_7X7,
    DOT_PIXEL_8X8,
} DOT_PIXEL;

typedef enum {
    DOT_FILL_AROUND = 1,
    DOT_FILL_RIGHTUP,
} DOT_STYLE;

typedef enum {
    LINE_SOLID = 0,
    LINE_DOTTED,
} LINE_STYLE;

typedef enum {
    DRAW_EMPTY = 0,
    DRAW_FULL,
} DRAW_FILL;

typedef struct {
    uint16_t Year;
    uint8_t  Month;
    uint8_t  Day;
    uint8_t  Hour;
    uint8_t  Min;
    uint8_t  Sec;
} DEV_TIME;

typedef enum {
    L2R_U2D = 0,
    L2R_D2U,
    R2L_U2D,
    R2L_D2U,

    U2D_L2R,
    U2D_R2L,
    D2U_L2R,
    D2U_R2L,
} LCD_SCAN_DIR;

typedef struct {
    POINT Xpoint0;
    POINT Ypoint0;
    POINT Xpoint;
    POINT Ypoint;
    unsigned char chStatus;
    unsigned char chType;
    int iXoff;
    int iYoff;
    float fXfac;
    float fYfac;
    LCD_SCAN_DIR TP_Scan_Dir;
}TP_DEV;

typedef struct{
    POINT Xpoint;
    POINT Ypoint;
    COLOR Color;
    DOT_PIXEL DotPixel; 
}TP_DRAW;

typedef struct {
    LENGTH LCD_Dis_Column;
    LENGTH LCD_Dis_Page;
    LCD_SCAN_DIR LCD_Scan_Dir;
    POINT LCD_X_Adjust;
    POINT LCD_Y_Adjust;
} LCD_DIS;

typedef struct __attribute__((packed, aligned(1))) rut_structure{
    char rut[10];
    uint32_t index;
} rut_t;

typedef struct __attribute__((packed, aligned(1))) card_structure{
    uint8_t cardType;
    uint8_t permisos;
    uint8_t antipassbackStatus;
    uint16_t canalHorario;
    uint32_t code1;
    uint32_t code2;
    uint32_t index;
} card_t;

typedef struct __attribute__((packed, aligned(1))) reservation_structure{
    char qr[8];
    char code[6];
    uint32_t code1;
    uint32_t code2;
    uint64_t init_time;
    uint64_t end_time;
    uint32_t index;
} reservation_t;

typedef union ip4_str
{
    ip4_addr_t ip_addr_i;
    struct { uint8_t addr[4]; };
} ip4_str;

typedef struct wifi_context
{
    char *wifi_password;
    int8_t *opt;
    uint16_t *ap_count;
    wifi_ap_record_t *ap_info;
    uint8_t *http_ind;
} wifi_context_t;

typedef struct reg_context
{
    char *registration_code;
    char *opt_web;
    uint8_t *http_ind;
} reg_context_t;

extern sFONT Font24;
extern sFONT Font20;
extern sFONT Font16;
extern sFONT Font12;
extern sFONT Font8;

extern sFONT dreamit_LOGO_Top;
extern sFONT dreamit_LOGO_Bot;

extern sFONT dreamit_LOGO_Big_Top;
extern sFONT dreamit_LOGO_Big_Bot;
extern sFONT dreamit_LOGO_Big_Text;

extern sFONT cross_Sign;
extern sFONT check_Sign;
extern sFONT circle_Sign;
extern sFONT button_Sign;

extern DEV_TIME sDev_time;
extern LCD_DIS sLCD_DIS;

extern uint8_t touch_context_status;

extern char apitoken[30];
extern char database[20];
extern char idcontrolador[3];
extern char wifi_ssid[30];
extern char screen_qr[7];

extern ip4_str ip_addr;
extern ip4_str gw_addr;
extern uint8_t mac[6];

extern time_t system_now;

extern uint8_t  loaded_data;
extern uint32_t rut_size;
extern uint32_t card_size;
extern uint32_t reservation_size;
extern uint64_t timestamp;
extern uint64_t timestamp_temp;

extern RUT rut_importer[COPY_SIZE];
extern RUT rut_data[RUT_READER_SIZE];

extern CARD card_importer[COPY_SIZE];
extern CARD card_data[CARD_READER_SIZE];

extern RESERVATION reservation_importer[COPY_SIZE];
extern RESERVATION reservation_data[RESERVATION_READER_SIZE];

extern xQueueHandle ntp_task_queue;
extern xQueueHandle rgb_task_queue;
extern xQueueHandle relay_task_queue;
extern xQueueHandle buzzer_task_queue;
extern xQueueHandle screen_task_queue;

extern SemaphoreHandle_t rut_semaphore;
extern SemaphoreHandle_t card_semaphore;
extern SemaphoreHandle_t reservation_semaphore;

extern TaskHandle_t ntp_task_handle;
extern TaskHandle_t rgb_task_handle;
extern TaskHandle_t data_task_handle;
extern TaskHandle_t relay_task_handle;
extern TaskHandle_t buzzer_task_handle;
extern TaskHandle_t screen_task_handle;
extern TaskHandle_t wiegand_task_handle;
extern TaskHandle_t debounce_task_handle;

extern screen_queue_t screen_task_data;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

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

#define SCREEN_SIGNAL(screen_msg, screen_status ,screen_s) {\
                    screen_queue_t screen = {\
                        .msg    = "",\
                        .status = screen_status,\
                        .timer  = screen_s\
                    };\
                    strcpy(screen.msg, screen_msg);\
                    xQueueSend(screen_task_queue, &screen, (unsigned int) 0);\
                    }
