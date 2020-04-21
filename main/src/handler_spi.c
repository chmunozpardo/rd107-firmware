#include "handler_spi.h"

static const char* TAG = "spi_handler";

static esp_err_t ret;

static spi_device_handle_t spi_rgb;
static spi_device_handle_t spi_touch;
static spi_device_handle_t spi_screen;

static spi_transaction_t t_rgb;
static spi_transaction_t t_rgb_delay;
static spi_transaction_t t_screen_8b;
static spi_transaction_t t_screen_16b;

static uint32_t tx_data_8b[3] = {0};
static uint32_t rx_data_8b[3] = {0};

DRAM_ATTR uint8_t data_rgb                    =  0 ;
DRAM_ATTR uint8_t array_rgb[24]               = {0};
DRAM_ATTR uint16_t tx_data_16b[SCREEN_BUFFER] = {0};

void spi_init()
{

    rgb_task_queue     = xQueueCreate(10, sizeof(uint8_t)*5);
    screen_task_queue  = xQueueCreate(10, sizeof(uint8_t)*6);

    memset(&t_rgb, 0, sizeof(t_rgb));
    t_rgb.length    = 8 * RGB_DATA_N;
    t_rgb.tx_buffer = &array_rgb;
    t_rgb.user      = (void*)0;

    memset(&t_rgb_delay, 0, sizeof(t_rgb_delay));
    t_rgb_delay.length    = 8;
    t_rgb_delay.tx_buffer = &data_rgb;
    t_rgb_delay.user      = (void*)0;

    memset(&t_screen_8b, 0, sizeof(t_screen_8b));
    t_screen_8b.length    = 8 * 3;
    t_screen_8b.user      = (void*)0;
    t_screen_8b.tx_buffer = tx_data_8b;
    t_screen_8b.rx_buffer = rx_data_8b;

    memset(&t_screen_16b, 0, sizeof(t_screen_16b));
    t_screen_16b.length    = SCREEN_BUFFER * 16;
    t_screen_16b.rxlength  = 0;
    t_screen_16b.user      = (void*)0;
    t_screen_16b.tx_buffer = tx_data_16b;

    spi_bus_config_t buscfg_rgb=
    {
        .miso_io_num     = -1,
        .mosi_io_num     = PIN_NUM_MOSI,
        .sclk_io_num     = -1,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = RGB_DATA_N*RGB_LEDS
    };

    spi_device_interface_config_t devcfg_rgb=
    {
        .clock_speed_hz = 6400000,
        .mode           = 0,
        .spics_io_num   = -1,
        .queue_size     = 40,
    };

    spi_bus_config_t buscfg_screen=
    {
        .miso_io_num     = LCD_PIN_MISO,
        .mosi_io_num     = LCD_PIN_MOSI,
        .sclk_io_num     = LCD_PIN_CLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = 0,
    };

    spi_device_interface_config_t devcfg_screen=
    {
        .clock_speed_hz = LCD_FREQ,
        .mode           = 0,
        .spics_io_num   = -1,
        .queue_size     = 40,
    };

    spi_device_interface_config_t devcfg_touch=
    {
        .clock_speed_hz = TOUCH_FREQ,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 1,
    };

    ESP_LOGI(TAG, "Initializing SPI for RGB");
    ret = spi_bus_initialize(VSPI_HOST, &buscfg_rgb, 2);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(VSPI_HOST, &devcfg_rgb, &spi_rgb);
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Initializing SPI for Screen & Touch");
    ret = spi_bus_initialize(HSPI_HOST, &buscfg_screen, 1);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(HSPI_HOST, &devcfg_screen, &spi_screen);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(HSPI_HOST, &devcfg_touch, &spi_touch);
    ESP_ERROR_CHECK(ret);

    gpio_pad_select_gpio(LCD_PIN_DC);
    gpio_set_direction(LCD_PIN_DC, GPIO_MODE_OUTPUT);
    gpio_set_level(LCD_PIN_DC, 1);

    gpio_pad_select_gpio(LCD_PIN_CS);
    gpio_set_direction(LCD_PIN_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(LCD_PIN_CS, 1);

    gpio_pad_select_gpio(TOUCH_PIN_CS);
    gpio_set_direction(TOUCH_PIN_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(TOUCH_PIN_CS, 1);
}

void rgb_spi_transmit(void)
{
    ret = spi_device_polling_transmit(spi_rgb, &t_rgb);
}

void rgb_spi_delay(void)
{
    for(int j = 0; j < RGB_RESET_TIME; j++) ret = spi_device_polling_transmit(spi_rgb, &t_rgb_delay);
}

void screen_write_byte(uint8_t Data)
{
    t_screen_8b.length   = 8;
    t_screen_8b.rxlength = 0;
    tx_data_8b[0] = Data;
    gpio_set_level(LCD_PIN_DC, 0);
    gpio_set_level(LCD_PIN_CS, 0);
    ret = spi_device_polling_transmit(spi_screen, &t_screen_8b);
    gpio_set_level(LCD_PIN_CS, 1);
}

void screen_write_word(uint16_t Data, uint32_t DataLen)
{
    uint32_t i, j;
    uint32_t rem = DataLen % SCREEN_BUFFER;
    uint32_t mul = DataLen / SCREEN_BUFFER;
    gpio_set_level(LCD_PIN_DC, 1);
    gpio_set_level(LCD_PIN_CS, 0);
    for(j = 0; j < mul; j++)
    {
        for(i = 0; i < SCREEN_BUFFER; i++) tx_data_16b[i] = SPI_SWAP_DATA_TX(Data, 16);
        t_screen_16b.length   = 16 * SCREEN_BUFFER;
        ret = spi_device_polling_transmit(spi_screen, &t_screen_16b);
    }
    if(rem > 0)
    {
        t_screen_16b.length = 16 * rem;
        for(i = 0; i < rem; i++) tx_data_16b[i] = SPI_SWAP_DATA_TX(Data, 16);
        ret = spi_device_polling_transmit(spi_screen, &t_screen_16b);
    }
    gpio_set_level(LCD_PIN_CS, 1);
}

uint16_t screen_read_byte(uint8_t Data)
{
    uint8_t a0           = 0;
    uint8_t a1           = 0;
    t_screen_8b.length   = 8 * 3;
    t_screen_8b.rxlength = 8 * 3;

    tx_data_8b[0] = Data;
    tx_data_8b[1] = 0xFF;
    tx_data_8b[2] = 0xFF;

    gpio_set_level(TOUCH_PIN_CS, 0);
    ret = spi_device_polling_transmit(spi_touch, &t_screen_8b);
    gpio_set_level(TOUCH_PIN_CS, 1);
    uint32_t asdf = SPI_SWAP_DATA_RX(rx_data_8b[0], 24);
    uint16_t out = ((asdf>>8)<<5) | ((asdf>>16)>>3);
    return out;
}