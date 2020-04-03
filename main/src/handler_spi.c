#include "handler_spi.h"

static const char* TAG = "spi_handler";

static esp_err_t ret;
static spi_device_handle_t spi_rgb;
static spi_device_handle_t spi_screen;
static spi_transaction_t t_rgb;
static spi_transaction_t t_rgb_delay;
static spi_transaction_t t_screen;

DRAM_ATTR uint8_t array_rgb[24] = {0};
uint8_t data_rgb                =  0 ;

static uint8_t tx_data[3] = {0};
static uint8_t rx_data[3] = {0};

void spi_init()
{

    memset(&t_rgb, 0, sizeof(t_rgb));
    t_rgb.length = 8*RGB_DATA_N;
    t_rgb.tx_buffer = &array_rgb;
    t_rgb.user = (void*)0;

    memset(&t_rgb_delay, 0, sizeof(t_rgb_delay));
    t_rgb_delay.length = 8;
    t_rgb_delay.tx_buffer = &data_rgb;
    t_rgb_delay.user = (void*)0;

    spi_bus_config_t buscfg_rgb=
    {
        .miso_io_num = -1,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = RGB_DATA_N*RGB_LEDS
    };

    spi_device_interface_config_t devcfg_rgb=
    {
        .clock_speed_hz = 6400000,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 40,
    };

    ESP_LOGI(TAG, "Initializing SPI for RGB");
    ret = spi_bus_initialize(VSPI_HOST, &buscfg_rgb, 1);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(VSPI_HOST, &devcfg_rgb, &spi_rgb);
    ESP_ERROR_CHECK(ret);

    memset(&t_screen, 0, sizeof(t_screen));
    t_screen.length = 8 * 3;
    t_screen.user = (void*)0;
    t_screen.tx_buffer = tx_data;
    t_screen.rx_buffer = rx_data;

    spi_bus_config_t buscfg_screen=
    {
        .miso_io_num = LCD_PIN_MISO,
        .mosi_io_num = LCD_PIN_MOSI,
        .sclk_io_num = LCD_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 1,
    };

    spi_device_interface_config_t devcfg_screen=
    {
        .clock_speed_hz = LCD_FREQ,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 1,
    };

    ESP_LOGI(TAG, "Initializing SPI for TOUCH");
    ret = spi_bus_initialize(HSPI_HOST, &buscfg_screen, 0);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(HSPI_HOST, &devcfg_screen, &spi_screen);
    ESP_ERROR_CHECK(ret);

    gpio_pad_select_gpio(LCD_PIN_DC);
    gpio_set_direction(LCD_PIN_DC, GPIO_MODE_OUTPUT);
    gpio_set_level(LCD_PIN_DC, 1);

    gpio_pad_select_gpio(LCD_PIN_CS);
    gpio_set_direction(LCD_PIN_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(LCD_PIN_CS, 1);

    gpio_pad_select_gpio(LCD_PIN_IRQ);
    gpio_set_direction(LCD_PIN_IRQ, GPIO_MODE_INPUT);

    gpio_pad_select_gpio(LCD_PIN_BUSY);
    gpio_set_direction(LCD_PIN_BUSY, GPIO_MODE_INPUT);
}

void rgb_spi_transmit(void)
{
    ret = spi_device_polling_transmit(spi_rgb, &t_rgb);
}

void rgb_spi_delay(void)
{
    for(int j = 0; j < RGB_RESET_TIME; j++)
    {
        ret = spi_device_polling_transmit(spi_rgb, &t_rgb_delay);
    }
}

void SPI4W_Write_Byte(uint8_t Data)
{
    t_screen.length = 8 * 1;
    tx_data[0] = Data;
    ret = spi_device_polling_transmit(spi_screen, &t_screen);
}

uint16_t SPI4W_Read_Byte(uint8_t Data)
{
    uint8_t a0 = 0;
    uint8_t a1 = 0;

    t_screen.length = 8 * 1;

    gpio_set_level(LCD_PIN_CS, 0);
    tx_data[0] = Data;
    ret = spi_device_polling_transmit(spi_screen, &t_screen);

    tx_data[0] = 0xFF;
    ret = spi_device_polling_transmit(spi_screen, &t_screen);
    a0 = rx_data[0];

    tx_data[0] = 0xFF;
    ret = spi_device_polling_transmit(spi_screen, &t_screen);
    a1 = rx_data[0];
    gpio_set_level(LCD_PIN_CS, 1);
    uint16_t out = (a0 << 5) | (a1 >> 3);
    return out;
}