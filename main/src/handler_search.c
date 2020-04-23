#include "handler_search.h"

static const char *TAG = "search_handler";

static DRAM_ATTR CARD inputCard     =  {0};
static DRAM_ATTR bool status        =    0;
static DRAM_ATTR uint32_t read_size =    0;
static FILE *f                      = NULL;

void IRAM_ATTR search_card(uint8_t size, uint64_t value)
{
    status = 0;
    RGB_SIGNAL(RGB_ORANGE, RGB_LEDS, 0);
    if(size == 26)
    {
        inputCard.cardType = 2;
        inputCard.code1    = HID_CODE1(value);
        inputCard.code2    = HID_CODE2(value);
    }
    else if(size == 32)
    {
        inputCard.cardType = 5;
        inputCard.code1    = MIFARE(value);
        inputCard.code2    = 0;
    }
    else
    {
        inputCard.cardType = size;
        inputCard.code1    = (value >> 32) & 0xFFFFFFFF;
        inputCard.code2    = (value >>  0) & 0xFFFFFFFF;
    }

    xSemaphoreTake(reg_semaphore, portMAX_DELAY);
    f = fopen(FILE_CARDS, "r");
    ESP_LOGD(TAG, "Searching card %u, [%u,%u] in database", inputCard.cardType, inputCard.code1, inputCard.code2);

    if(f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        f = fopen(FILE_CARDS, "w+");
    }

    while((read_size = fread(card_data, CARD_SIZE, CARD_READER_SIZE, f)) > 0)
    {
        for(int i = 0; i < read_size; i++)
        {
            status = CARD_COMPARE(inputCard, card_data[i]);
            if(status) break;
        }
    }

    fclose(f);
    xSemaphoreGive(reg_semaphore);

    if(status)
    {
        ESP_LOGD(TAG, " --> Card found <--");
        RELAY_SIGNAL(DISPLAY_TIME);
        SCREEN_SIGNAL("GOOD", 1, DISPLAY_TIME);
        RGB_SIGNAL(RGB_GREEN, RGB_LEDS, DISPLAY_TIME);
    }
    else
    {
        ESP_LOGD(TAG, " --> Card not found <--");
        SCREEN_SIGNAL("BAD", 1, DISPLAY_TIME);
        RGB_SIGNAL(RGB_RED, RGB_LEDS, DISPLAY_TIME);
    }
    RGB_SIGNAL(RGB_CYAN, RGB_LEDS, 0);
}

void IRAM_ATTR search_reservation_qr(char *qr)
{
    status = 1;
    RGB_SIGNAL(RGB_ORANGE, RGB_LEDS, 0);
    xSemaphoreTake(reservation_semaphore, portMAX_DELAY);
    f = fopen(FILE_RESERVATIONS, "r");
    ESP_LOGD(TAG, "Searching reservation QR %s in database", qr);

    if(f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        f = fopen(FILE_RESERVATIONS, "w+");
    }

    while((read_size = fread(reservation_data, RESERVATION_SIZE, RESERVATION_READER_SIZE, f)) > 0)
    {
        for(int i = 0; i < read_size; i++)
        {
            status = RESERVATION_COMPARE_QR(qr, reservation_data[i]);
            if(status == 0) break;
        }
    }

    fclose(f);
    xSemaphoreGive(reservation_semaphore);

    if(status == 0)
    {
        ESP_LOGD(TAG, " --> Reservation found <--");
        RELAY_SIGNAL(DISPLAY_TIME);
        SCREEN_SIGNAL("GOOD", 1, DISPLAY_TIME);
        RGB_SIGNAL(RGB_GREEN, RGB_LEDS, DISPLAY_TIME);
    }
    else
    {
        ESP_LOGD(TAG, " --> Reservation not found <--");
        SCREEN_SIGNAL("BAD", 1, DISPLAY_TIME);
        RGB_SIGNAL(RGB_RED, RGB_LEDS, DISPLAY_TIME);
    }
    RGB_SIGNAL(RGB_CYAN, RGB_LEDS, 0);
}

void IRAM_ATTR search_reservation_code(char *code)
{
    status = 1;
    RGB_SIGNAL(RGB_ORANGE, RGB_LEDS, 0);
    xSemaphoreTake(reservation_semaphore, portMAX_DELAY);
    f = fopen(FILE_RESERVATIONS, "r");
    ESP_LOGD(TAG, "Searching reservation code %s in database", code);

    if(f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        f = fopen(FILE_RESERVATIONS, "w+");
    }

    while((read_size = fread(reservation_data, RESERVATION_SIZE, RESERVATION_READER_SIZE, f)) > 0)
    {
        for(int i = 0; i < read_size; i++)
        {
            status = RESERVATION_COMPARE_CODE(code, reservation_data[i]);
            if(status == 0) break;
        }
    }

    fclose(f);
    xSemaphoreGive(reservation_semaphore);

    if(status == 0)
    {
        ESP_LOGD(TAG, " --> Reservation found <--");
        RELAY_SIGNAL(DISPLAY_TIME);
        SCREEN_SIGNAL("GOOD", 1, DISPLAY_TIME);
        RGB_SIGNAL(RGB_GREEN, RGB_LEDS, DISPLAY_TIME);
    }
    else
    {
        ESP_LOGD(TAG, " --> Reservation not found <--");
        SCREEN_SIGNAL("BAD", 1, DISPLAY_TIME);
        RGB_SIGNAL(RGB_RED, RGB_LEDS, DISPLAY_TIME);
    }
    RGB_SIGNAL(RGB_CYAN, RGB_LEDS, 0);
}