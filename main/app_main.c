#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "tcpip_adapter.h"

#include "connect.h"
#include "parse_handler.h"
#include "data_handler.h"
#include "fs_handler.h"
#include "rgb_handler.h"
#include "wiegand_handler.h"
#include "buzzer_handler.h"
#include "card_handler.h"

#define REG_READER_SIZE 1000

char timestamp[100]              = "0";
card_structure *registers_data = NULL;
int registers_size               = 0;

void print_registers(void){
    printf("Timestamp = %s, Registers size = %d\n", timestamp, registers_size);
    FILE *f = fopen(REG_FILE,"r");
    int read_size = 0;
    while((read_size=fread(registers_data, CARD_FULL_SIZE, REG_READER_SIZE, f)) > 0){
        for(int i = 0; i < read_size; i++){
            printf("Struct values:");
            printf(" cardType = %u", registers_data[i].cardType);
            printf(", code = [%u, %u]\n", registers_data[i].code1, registers_data[i].code2);
        }
    }
    fclose(f);
}

void app_main(){

    registers_data = (card_structure *) malloc(REG_READER_SIZE * CARD_FULL_SIZE);

    rgb_init();
    buzzer_init();
    rgb_fixed_leds(RGB_RED);
    fs_init();
    wiegand_init();

    unlink(REG_FILE);
    unlink(REG_FILE_JSON);
    unlink(REG_TIMESTAMP);

    rgb_fixed_leds(RGB_YELLOW);
    ESP_ERROR_CHECK( nvs_flash_init() );
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_create_default() );
    ESP_ERROR_CHECK( wifi_connect() );

    rgb_rainbow_leds();

    data_load();
    //print_registers();

    xTaskCreate(wiegand_read, "wiegand_read", 2048, NULL, 10, NULL);
}
