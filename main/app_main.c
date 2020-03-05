#include "connect.h"
#include "parse_handler.h"
#include "data_handler.h"
#include "fs_handler.h"
#include "rgb_handler.h"
#include "wiegand_handler.h"
#include "buzzer_handler.h"
#include "card_handler.h"

uint64_t timestamp              =    0;
CARD *registers_data            = NULL;
uint32_t registers_size         =    0;
xQueueHandle rgb_task_queue     = NULL;
SemaphoreHandle_t reg_semaphore = NULL; 
SemaphoreHandle_t rgb_semaphore = NULL;
SemaphoreHandle_t wiegand_semaphore = NULL;

void print_registers(void)
{
    struct timeval now;
    float current_time =  0;
    gettimeofday(&now, NULL);
    current_time = now.tv_sec + now.tv_usec/1000000.0;
    printf("Timestamp = %llu, Registers size = %d\n", timestamp, registers_size);
    FILE *f = fopen(REG_FILE,"r");
    int read_size = 0;
    while((read_size=fread(registers_data, CARD_FULL_SIZE, CARD_READER_SIZE, f)) > 0)
    {
        for(int i = 0; i < read_size; i++)
        {
            printf("Card = %u, %u, %u\n", registers_data[i].cardType, registers_data[i].code1, registers_data[i].code2);
        }
        printf("Read size = %d\n", read_size);
    }
    fclose(f);
    gettimeofday(&now, NULL);
    current_time = now.tv_sec + now.tv_usec/1000000.0 - current_time;
    printf("Elapsed time = %f\n", current_time);
}

static void setup()
{
    registers_data    = (CARD *) malloc(CARD_READER_SIZE * CARD_FULL_SIZE);
    reg_semaphore     = xSemaphoreCreateMutex();
    rgb_semaphore     = xSemaphoreCreateMutex();
    wiegand_semaphore = xSemaphoreCreateMutex();

    fs_init();
    remove(REG_FILE);
    remove(REG_FILE_JSON);
    remove(REG_TIMESTAMP);

    rgb_init();
    buzzer_init();
    xTaskCreatePinnedToCore(rgb_task    , "rgb_controller", 2048, NULL, 20, NULL, 0);

    wiegand_init();

    RGB_SIGNAL(RGB_RED, RGB_LEDS, 0);

    nvs_flash_init();
    tcpip_adapter_init();
    esp_event_loop_create_default();
    wifi_connect();

    xTaskCreatePinnedToCore(wiegand_read, "wiegand_read", 2048, NULL,  0, NULL, 1);
    xTaskCreatePinnedToCore(data_request, "data_request", 8192, NULL, 10, NULL, 0);
}

void app_main()
{
    setup();
}
