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

bool card_compare(const simple_structure a, const simple_structure b){
    return a.cardType == b.cardType && a.code1 == b.code1 && a.code2 == b.code2;
}

void card_search(uint8_t size, uint64_t value){
    simple_structure inputCard;
    if(size == 26){
        inputCard.cardType = 2;
        inputCard.code1 = (value >> (17 + 6)) & 0xFF;
        inputCard.code2 = (value >> ( 1 + 6)) & 0xFFFF;
    }
    else{
        inputCard.cardType = 5;
        inputCard.code1 = (value & 0x000000FF) << 24 |
                          (value & 0x0000FF00) <<  8 |
                          (value & 0x00FF0000) >>  8 |
                          (value & 0xFF000000) >> 24;
        inputCard.code2 = 0;
    }
    bool status = 0;
    for(int i = 0; i < registers_size; i++){
        status = card_compare(inputCard, registers_data[i]);
        if(status) break;
    }
    card_action(status);
}

void card_action(bool arg){
    if(arg){
        rgb_fixed_leds(RGB_GREEN);
        vTaskDelay(1000/portTICK_PERIOD_MS);
        rgb_fixed_leds(RGB_IDLE);
    }
    else{
        rgb_fixed_leds(RGB_RED);
        vTaskDelay(1000/portTICK_PERIOD_MS);
        rgb_fixed_leds(RGB_IDLE);
    }
}