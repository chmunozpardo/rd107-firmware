#include "handler_parse.h"
#include "handler_data.h"
#include "handler_files.h"
#include "handler_rgb.h"
#include "handler_wiegand.h"
#include "handler_buzzer.h"
#include "handler_relay.h"
#include "handler_card.h"
#include "handler_screen.h"
#include "handler_touch.h"
#include "handler_spi.h"
#include "Waveshare_ILI9486.h"

uint32_t registers_size = 0;
uint64_t timestamp      = 0;

xQueueHandle rgb_task_queue    = NULL;
xQueueHandle relay_task_queue  = NULL;
xQueueHandle buzzer_task_queue = NULL;

TaskHandle_t rgb_task_handle     = NULL;
TaskHandle_t relay_task_handle   = NULL;
TaskHandle_t buzzer_task_handle  = NULL;
TaskHandle_t data_task_handle    = NULL;
TaskHandle_t wiegand_task_handle = NULL;

SemaphoreHandle_t reg_semaphore    = NULL;

DRAM_ATTR CARD data_importer[COPY_SIZE]         = {0};
DRAM_ATTR CARD registers_data[CARD_READER_SIZE] = {0};

static void setup()
{
    fs_init();
    rgb_init();
    spi_init();
    relay_init();
    buzzer_init();
    wiegand_init();

    reg_semaphore    = xSemaphoreCreateMutex();

    LCD_SCAN_DIR Lcd_ScanDir = SCAN_DIR_DFT;
    LCD_Init( Lcd_ScanDir, 200);
    LCD_Clear(WHITE);
    GUI_QR("Hola");

    remove(REG_FILE);
    remove(REG_FILE_JSON);
    remove(REG_TIMESTAMP);

    xTaskCreatePinnedToCore(rgb_task    , "rgb_task    ", 2048, NULL, 1,  &rgb_task_handle    , 0);
    xTaskCreatePinnedToCore(relay_task  , "rly_task    ", 2048, NULL, 1,  &relay_task_handle  , 0);
    xTaskCreatePinnedToCore(buzzer_task , "bzr_task    ", 2048, NULL, 1,  &buzzer_task_handle , 0);

    RGB_SIGNAL(RGB_RED, RGB_LEDS, 0);

    nvs_flash_init();
    tcpip_adapter_init();
    esp_event_loop_create_default();
    wifi_connect();

    xTaskCreatePinnedToCore(data_task   , "data_task   ", 4096, NULL, 1,  &data_task_handle   , 0);
    xTaskCreatePinnedToCore(wiegand_task, "wiegand_task", 2048, NULL, 0,  &wiegand_task_handle, 1);
}

void app_main()
{
    setup();
}
