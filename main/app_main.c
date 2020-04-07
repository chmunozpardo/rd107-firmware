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
#include "handler_sntp.h"
#include "handler_wifi.h"
#include "Waveshare_ILI9486.h"

static const char* TAG = "app_main";

uint32_t registers_size = 0;
uint64_t timestamp      = 0;

char screen_qr[6] = "000000";

xQueueHandle qr_task_queue     = NULL;
xQueueHandle rgb_task_queue    = NULL;
xQueueHandle relay_task_queue  = NULL;
xQueueHandle buzzer_task_queue = NULL;

TaskHandle_t qr_task_handle      = NULL;
TaskHandle_t rgb_task_handle     = NULL;
TaskHandle_t relay_task_handle   = NULL;
TaskHandle_t buzzer_task_handle  = NULL;
TaskHandle_t data_task_handle    = NULL;
TaskHandle_t wiegand_task_handle = NULL;

SemaphoreHandle_t reg_semaphore = NULL;

DRAM_ATTR CARD data_importer[COPY_SIZE]         = {0};
DRAM_ATTR CARD registers_data[CARD_READER_SIZE] = {0};

static void setup()
{
    // Disable Watchdog at startup
    esp_task_wdt_deinit();

    // Registers file semaphore
    reg_semaphore    = xSemaphoreCreateMutex();

    // First load filesystem
    fs_init();

    // Remove data file
    remove(REG_FILE);
    remove(REG_FILE_JSON);
    remove(REG_TIMESTAMP);

    // Initiate all peripherals
    rgb_init();
    spi_init();
    relay_init();
    buzzer_init();
    wiegand_init();

    //
    LCD_SCAN_DIR Lcd_ScanDir = SCAN_DIR_DFT;
    LCD_Init( Lcd_ScanDir, 200);
    LCD_Clear(WHITE);

    // Initiate WiFi configurations
    wifi_init();
    ntp_init();

    // Register device
    data_register();

    // --------------------------------------------
    // Enable Watchdog after all the configurations
    esp_task_wdt_init(CONFIG_ESP_TASK_WDT_TIMEOUT_MS, false);
    esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(0));
    esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(1));

    // 
    xTaskCreatePinnedToCore(qr_task     , "cqr_task", 4096, NULL, 1,  &qr_task_handle     , 0);
    xTaskCreatePinnedToCore(rgb_task    , "rgb_task", 2048, NULL, 1,  &rgb_task_handle    , 0);
    xTaskCreatePinnedToCore(relay_task  , "rly_task", 2048, NULL, 1,  &relay_task_handle  , 0);
    xTaskCreatePinnedToCore(buzzer_task , "bzr_task", 2048, NULL, 1,  &buzzer_task_handle , 0);
    xTaskCreatePinnedToCore(data_task   , "dat_task", 4096, NULL, 1,  &data_task_handle   , 0);
    xTaskCreatePinnedToCore(wiegand_task, "wgn_task", 2048, NULL, 0,  &wiegand_task_handle, 1);
}

void app_main()
{
    setup();
}
