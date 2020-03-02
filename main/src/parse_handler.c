#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h> 
#include "parse_handler.h"
#include "tcpip_adapter.h"

static const char* TAG = "parse_handler";

extern int registers_size;
extern char timestamp[100];

#define DATA_INIT_BRACKET   0
#define ACCESS_INIT_BRACKET 1
#define ARRAY_INIT_BRACKET  2
#define READ_STRUCT_INIT    3
#define READ_STRUCT_END     4
#define ARRAY_END_BRACKET   5
#define DATA_END_BRACKET    6
#define COUNT_INIT_BRACKET  7
/*
static void parse_insert_card(card_structure *prueba){
    if(strcmp(timestamp, "0") == 0){
        FILE *g = fopen(REG_FILE, "a");
        fwrite(prueba, CARD_FULL_SIZE, 1, g);
        fclose(g);
        registers_size++;
    }
    else{
        for(int i = 0; i < registers_size; i++){
            if(CARD_COMPARE((*prueba), registers_data[i])){
                FILE *g = fopen(REG_FILE, "a+");
                fseek(g, CARD_FULL_SIZE * i, SEEK_SET);
                fwrite(prueba, CARD_FULL_SIZE, 1, g);
                fclose(g);
                return;
            }
        }
        FILE *g = fopen(REG_FILE, "a+");
        fwrite(prueba, CARD_FULL_SIZE, 1, g);
        fclose(g);
        registers_size++;
    }
}*/

void parse_data(void){
    int new_regs          =  0;
    char line[100]        = "";
    float current_time    =  0;
    struct timeval now    = {0};
    uint8_t state         =  0;
    card_structure prueba = {0};

    ESP_LOGI(TAG, "Reading file");

    FILE *f;
    f = fopen(REG_FILE_JSON, "r+");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    gettimeofday(&now, NULL);
    current_time = now.tv_sec + now.tv_usec/1000000.0;

    while(1){
        if(state == DATA_INIT_BRACKET){
            fscanf(f, " {\"data\":{\"countData\":%d,\"accessRecords\":", &new_regs);
            state = READ_STRUCT_INIT;
        }
        else if(state == READ_STRUCT_INIT){
            ESP_LOGI(TAG, "Write pointer = %d", new_regs);
            uint32_t sweep_data = 0;
            uint32_t remaining = new_regs;
            registers_size = new_regs;
            FILE *g = fopen(REG_FILE, "w+");
            registers_size = new_regs;
            card_structure *registers_data_full = (card_structure *) malloc(CARD_FULL_SIZE * 1000);
            while(1){
                if(remaining >= 1000) sweep_data = 1000;
                else sweep_data = remaining;
                remaining = remaining - sweep_data;
                fread(registers_data_full, CARD_FULL_SIZE, sweep_data, f);
                fwrite(registers_data_full, CARD_FULL_SIZE, sweep_data, g);
                if(remaining == 0) break;
            };
            fclose(g);
            free(registers_data_full);
                //parse_insert_card(&prueba);
            //}
            state = READ_STRUCT_END;
        }
        else if(state == READ_STRUCT_END){
            uint32_t ctm = 0;
            fscanf(f, " ,\"currentTimestamp\":%u},\"estado\":%s}", &ctm, line);
            sprintf(timestamp, "%u", ctm);
            ESP_LOGI(TAG, " - %s - %s - ", timestamp, line);
            FILE *g = fopen(REG_TIMESTAMP, "w");
            fprintf(g, "%s %d", timestamp, registers_size);
            fclose(g);
            break;
        }
    }
    ESP_LOGI(TAG, "Closing file");
    fclose(f);

    gettimeofday(&now, NULL);
    current_time = now.tv_sec + now.tv_usec/1000000.0 - current_time;
    printf("Elapsed time = %f\n", current_time);

    struct stat st;
    if(stat(REG_TMP_FILE, &st) == 0)
        remove(REG_TMP_FILE);
}