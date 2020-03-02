#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "card_handler.h"
#include "rgb_handler.h"
#include "buzzer_handler.h"

static const char *TAG = "card_handler";

extern simple_structure *registers_data;
extern int registers_size;

void card_search(uint8_t size, uint64_t value){
    simple_structure inputCard;
    if(size == 26){
        inputCard.cardType = 2;
        inputCard.code1 = HID_CODE1(value);
        inputCard.code2 = HID_CODE2(value);
    }
    else{
        inputCard.cardType = 5;
        inputCard.code1 = MIFARE(value);
        inputCard.code2 = 0;
    }
    bool status = 0;
    printf("Inputcard = %d, %u, %u\n", inputCard.cardType, inputCard.code1, inputCard.code2);
    for(int i = 0; i < registers_size; i++){
        status = CARD_COMPARE(inputCard, registers_data[i]);
        if(status) break;
    }
    card_action(status);
}

void card_action(bool arg){
    if(arg) rgb_fixed_leds(RGB_GREEN);
    else rgb_fixed_leds(RGB_RED);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    rgb_fixed_leds(RGB_IDLE);
}