#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wiegand_handler.h"
#include "card_handler.h"

static const char *TAG = "wiegand_handler";

static xQueueHandle wiegand_reader_queue = NULL;

static void IRAM_ATTR wiegand_isr(void* arg){
    uint8_t input_read[2] = {gpio_get_level(WIEGAND_D0), gpio_get_level(WIEGAND_D1)};
    xQueueSendFromISR(wiegand_reader_queue, &input_read, NULL);
}

void wiegand_init(void){
    ESP_LOGI(TAG, "Initializing Wiegand");
    wiegand_reader_queue = xQueueCreate(10, sizeof(uint8_t)*2);
    gpio_pad_select_gpio(WIEGAND_D0);
    gpio_pad_select_gpio(WIEGAND_D1);
    gpio_set_direction(WIEGAND_D0, GPIO_MODE_INPUT);
    gpio_set_direction(WIEGAND_D1, GPIO_MODE_INPUT);
    gpio_pullup_dis(WIEGAND_D0);
    gpio_pullup_dis(WIEGAND_D1);
    gpio_pulldown_dis(WIEGAND_D0);
    gpio_pulldown_dis(WIEGAND_D1);
    gpio_set_intr_type(WIEGAND_D0, GPIO_INTR_NEGEDGE);
    gpio_set_intr_type(WIEGAND_D1, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(WIEGAND_D0, wiegand_isr, (void*) NULL);
    gpio_isr_handler_add(WIEGAND_D1, wiegand_isr, (void*) NULL);
}

void wiegand_read(void *arg){
    uint8_t input_read[2] = {0, 0};
    uint8_t count = 0;
    uint64_t store_card = 0;
    while(1){
        if(xQueueReceive(wiegand_reader_queue, &input_read, 25/portTICK_PERIOD_MS)){
            store_card |= (input_read[0] << (31 - count));
            ++count;
        }
        else if(count > 0){
            card_search(count, store_card);
            count = 0;
            store_card = 0;
            input_read[0] = 0;
            input_read[1] = 0;
        }
    }
}