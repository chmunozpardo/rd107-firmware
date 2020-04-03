#include "handler_rgb.h"
#include "handler_spi.h"

static const char* TAG = "rgb_handler";

static uint8_t status[5] = {0};

extern uint8_t data_rgb ;
extern DRAM_ATTR uint8_t array_rgb[24];

static void rgb_ws2812(uint8_t r, uint8_t g, uint8_t b)
{
    for(int i = 0; i < 8; i++) array_rgb[i +  0] = ((g >> (7 - i)) & (0x01)) == 0x01 ? WS2812_ON : WS2812_OFF;
    for(int i = 0; i < 8; i++) array_rgb[i +  8] = ((r >> (7 - i)) & (0x01)) == 0x01 ? WS2812_ON : WS2812_OFF;
    for(int i = 0; i < 8; i++) array_rgb[i + 16] = ((b >> (7 - i)) & (0x01)) == 0x01 ? WS2812_ON : WS2812_OFF;
}

static void rgb_fixed_leds(uint8_t r, uint8_t g, uint8_t b, uint8_t leds)
{
    for(int j = 0; j < RGB_LEDS; j++)
    {
        if(j <= leds)
        {
            rgb_ws2812(r, g, b);
            rgb_spi_transmit();
        }
        else
        {
            rgb_ws2812(0, 0, 0);
            rgb_spi_transmit();
        }
    }
    rgb_spi_delay();
}

void rgb_init(void)
{
    rgb_task_queue = xQueueCreate(10, sizeof(uint8_t)*5);
}

void rgb_task(void *arg)
{
    while(1)
    {
        if(xQueueReceive(rgb_task_queue, &status, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "Setting LEDs value to [R, G, B] = [%u, %u, %u]", status[0], status[1], status[2]);
            rgb_fixed_leds(status[0], status[1], status[2], status[3]);
            if(status[4] != 0) vTaskDelay(status[4]*1000/portTICK_PERIOD_MS);
        }
    }
}