#include "handler_rgb.h"

static const char* TAG = "rgb_handler";

static esp_err_t ret;
static spi_device_handle_t spi;
static spi_transaction_t t;
static spi_transaction_t t_emp;

static uint8_t data                =  0 ;
static uint8_t status[5]           = {0};
static DRAM_ATTR uint8_t array[24] = {0};

static void rgb_ws2812(uint8_t r, uint8_t g, uint8_t b)
{
    for(int i = 0; i < 8; i++) array[i +  0] = ((g >> (7 - i)) & (0x01)) == 0x01 ? WS2812_ON : WS2812_OFF;
    for(int i = 0; i < 8; i++) array[i +  8] = ((r >> (7 - i)) & (0x01)) == 0x01 ? WS2812_ON : WS2812_OFF;
    for(int i = 0; i < 8; i++) array[i + 16] = ((b >> (7 - i)) & (0x01)) == 0x01 ? WS2812_ON : WS2812_OFF;
}

static void rgb_fixed_leds(uint8_t r, uint8_t g, uint8_t b, uint8_t leds)
{
    for(int j = 0; j < RGB_LEDS; j++)
    {
        if(j <= leds)
        {
            rgb_ws2812(r, g, b);
            ret = spi_device_polling_transmit(spi, &t);
        }
        else
        {
            rgb_ws2812(0, 0, 0);
            ret = spi_device_polling_transmit(spi, &t);
        }
    }
    for(int j = 0; j < RGB_RESET_TIME; j++)
    {
        ret = spi_device_polling_transmit(spi, &t_emp);
    }
}

void rgb_init(void)
{
    rgb_task_queue = xQueueCreate(10, sizeof(uint8_t)*5);

    memset(&t, 0, sizeof(t));
    t.length = 8*RGB_DATA_N;
    t.tx_buffer = &array;
    t.user = (void*)0;

    memset(&t_emp, 0, sizeof(t_emp));
    t_emp.length = 8;
    t_emp.tx_buffer = &data;
    t_emp.user = (void*)0;

    spi_bus_config_t buscfg=
    {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = RGB_DATA_N*RGB_LEDS
    };

    spi_device_interface_config_t devcfg=
    {
        .clock_speed_hz = 6400000,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 40,
    };

    ESP_LOGI(TAG, "Initializing SPI for RGB");
    ret = spi_bus_initialize(VSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(VSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
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