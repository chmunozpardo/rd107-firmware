#include "handler_parse.h"
#include "handler_data.h"
#include "handler_files.h"
#include "handler_rgb.h"
#include "handler_wiegand.h"
#include "handler_buzzer.h"
#include "handler_relay.h"
#include "handler_search.h"
#include "handler_screen.h"
#include "handler_touch.h"
#include "handler_spi.h"
#include "handler_sntp.h"
#include "handler_wifi.h"

//static const char* TAG = "app_main";

uint32_t reservation_size = 0;
uint32_t card_size        = 0;
uint64_t timestamp        = 0;
uint64_t timestamp_temp   = 0;

char screen_qr[6] = "000000";

xQueueHandle rgb_task_queue    = NULL;
xQueueHandle relay_task_queue  = NULL;
xQueueHandle buzzer_task_queue = NULL;
xQueueHandle screen_task_queue = NULL;

TaskHandle_t rgb_task_handle     = NULL;
TaskHandle_t data_task_handle    = NULL;
TaskHandle_t relay_task_handle   = NULL;
TaskHandle_t buzzer_task_handle  = NULL;
TaskHandle_t screen_task_handle  = NULL;
TaskHandle_t wiegand_task_handle = NULL;

SemaphoreHandle_t reg_semaphore         = NULL;
SemaphoreHandle_t reservation_semaphore = NULL;

DRAM_ATTR CARD card_importer[COPY_SIZE]              = {0};
DRAM_ATTR CARD card_data[CARD_READER_SIZE]           = {0};

DRAM_ATTR RESERVATION reservation_importer[COPY_SIZE]           = {0};
DRAM_ATTR RESERVATION reservation_data[RESERVATION_READER_SIZE] = {0};

static void setup()
{
    // Disable Watchdog at startup
    esp_task_wdt_deinit();

    // Registers file semaphore
    reg_semaphore         = xSemaphoreCreateMutex();
    reservation_semaphore = xSemaphoreCreateMutex();

    // First load filesystem
    fs_init();

    // Remove data files
    remove(FILE_CARDS);
    remove(FILE_JSON);
    remove(FILE_RESERVATIONS);
    remove(FILE_CONFIG);
    remove(FILE_TIMESTAMP);

    FILE *f  = fopen(FILE_CONFIG, "w");
    fprintf(f, "%s,%s,%s", "dreamit-testing-rd107-2020", "16", "GK2_Titanium");
    fclose(f);

    // Initiate all peripherals
    spi_init();
    relay_init();
    buzzer_init();
    screen_init();
    wiegand_init();

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
    xTaskCreatePinnedToCore(screen_task , "scr_task", 4096, NULL, 1,  &screen_task_handle , 0);
    xTaskCreatePinnedToCore(rgb_task    , "rgb_task", 2048, NULL, 1,  &rgb_task_handle    , 0);
    xTaskCreatePinnedToCore(relay_task  , "rly_task", 2048, NULL, 1,  &relay_task_handle  , 0);
    xTaskCreatePinnedToCore(buzzer_task , "bzr_task", 2048, NULL, 1,  &buzzer_task_handle , 0);
    xTaskCreatePinnedToCore(data_task   , "dat_task", 4096, NULL, 1,  &data_task_handle   , 1);
    xTaskCreatePinnedToCore(wiegand_task, "wgn_task", 2048, NULL, 0,  &wiegand_task_handle, 1);
}

void app_main()
{
    setup();
}
