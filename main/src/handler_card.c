#include "handler_parse.h"
#include "handler_card.h"
#include "handler_rgb.h"
#include "handler_buzzer.h"

static const char *TAG = "card_handler";

static DRAM_ATTR CARD inputCard     =  {0};
static DRAM_ATTR bool status        =    0;
static DRAM_ATTR uint32_t read_size =    0;
static FILE *f                      = NULL;

void IRAM_ATTR card_search(uint8_t size, uint64_t value)
{
    #ifdef DEBUG_INFO
        struct timeval now;
        float current_time =  0;
        gettimeofday(&now, NULL);
        current_time = now.tv_sec + now.tv_usec/1000000.0;
    #endif

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
    f = fopen(REG_FILE, "r");
    ESP_LOGI(TAG, "Searching card %u, [%u,%u] in database", inputCard.cardType, inputCard.code1, inputCard.code2);

    if(f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        xSemaphoreGive(reg_semaphore);
        return;
    }

    while((read_size = fread(registers_data, CARD_FULL_SIZE, CARD_READER_SIZE, f)) > 0)
    {
        for(int i = 0; i < read_size; i++)
        {
            status = CARD_COMPARE(inputCard, registers_data[i]);
            if(status) break;
        }
    }

    fclose(f);
    xSemaphoreGive(reg_semaphore);

    #ifdef DEBUG_INFO
        gettimeofday(&now, NULL);
        current_time = now.tv_sec + now.tv_usec/1000000.0 - current_time;
        ESP_LOGI(TAG, "Elapsed time = %f", current_time);
    #endif

    if(status)
    {
        ESP_LOGI(TAG, " --> Card found <--");
        RGB_SIGNAL(RGB_GREEN, RGB_LEDS, 1);
    }
    else
    {
        ESP_LOGI(TAG, " --> Card not found <--");
        RGB_SIGNAL(RGB_RED, RGB_LEDS, 1);
    }
    RGB_SIGNAL(RGB_CYAN, RGB_LEDS, 0);

}