#include "SPI_LCD.h"

static const char* TAG = "tft_handler";

static esp_err_t ret;
static spi_device_handle_t spi;
static spi_transaction_t t;

static uint8_t tx_data[3] = {0};
static uint8_t rx_data[3] = {0};

void LCD_SPI()
{
    memset(&t, 0, sizeof(t));
    t.length = 8 * 3;
    t.user = (void*)0;
    t.tx_buffer = tx_data;
    t.rx_buffer = rx_data;

    spi_bus_config_t buscfg=
    {
        .miso_io_num = LCD_PIN_MISO,
        .mosi_io_num = LCD_PIN_MOSI,
        .sclk_io_num = LCD_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 1,
    };

    spi_device_interface_config_t devcfg=
    {
        .clock_speed_hz = LCD_FREQ,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 1,
    };

    ESP_LOGI(TAG, "Initializing SPI for TOUCH");
    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 0);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
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

void SPI4W_Write_Byte(uint8_t Data)
{
    t.length = 8 * 1;
    tx_data[0] = Data;
    ret = spi_device_polling_transmit(spi, &t);
}

uint16_t SPI4W_Read_Byte(uint8_t Data)
{
    uint8_t a0 = 0;
    uint8_t a1 = 0;

    t.length = 8 * 1;

    gpio_set_level(LCD_PIN_CS, 0);
    for(int i = 0; i < 1000; i++) asm("nop");
    tx_data[0] = Data;
    ret = spi_device_polling_transmit(spi, &t);

    tx_data[0] = 0xFF;
    ret = spi_device_polling_transmit(spi, &t);
    a0 = rx_data[0];

    tx_data[0] = 0xFF;
    ret = spi_device_polling_transmit(spi, &t);
    a1 = rx_data[0];

    gpio_set_level(LCD_PIN_CS, 1);
    //printf("rx_data[0] = %x, rx_data[1] = %x, rx_data[2] = %x\n",
    //        rx_data[0]     , rx_data[1]     , rx_data[2]);
    uint16_t out = (a0 << 8) | a1;
    return out;
}