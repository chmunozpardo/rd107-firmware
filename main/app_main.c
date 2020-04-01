#include "parse_handler.h"
#include "data_handler.h"
#include "fs_handler.h"
#include "rgb_handler.h"
#include "wiegand_handler.h"
#include "buzzer_handler.h"
#include "card_handler.h"

#include "tft_handler.h"
#include "touch_handler.h"
#include "Waveshare_ILI9486.h"

uint32_t registers_size = 0;
uint64_t timestamp      = 0;

xQueueHandle rgb_task_queue = NULL;

TaskHandle_t rgb_task_handle     = NULL;
TaskHandle_t data_task_handle    = NULL;
TaskHandle_t wiegand_task_handle = NULL;

SemaphoreHandle_t reg_semaphore = NULL;
SemaphoreHandle_t rgb_semaphore = NULL;

DRAM_ATTR CARD data_importer[COPY_SIZE]         = {0};
DRAM_ATTR CARD registers_data[CARD_READER_SIZE] = {0};

static void setup()
{
    reg_semaphore = xSemaphoreCreateMutex();
    rgb_semaphore = xSemaphoreCreateMutex();

    LCD_SCAN_DIR Lcd_ScanDir = SCAN_DIR_DFT;
    LCD_Init( Lcd_ScanDir, 200);
    LCD_Clear(WHITE);
    GUI_QR("Hola");

    fs_init();
    remove(REG_FILE);
    remove(REG_FILE_JSON);
    remove(REG_TIMESTAMP);

    rgb_init();
    buzzer_init();
    wiegand_init();
    xTaskCreatePinnedToCore(rgb_task    , "rgb_task    ", 2048, NULL, 1,  &rgb_task_handle    , 0);

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
