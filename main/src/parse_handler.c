#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h> 
#include "parse_handler.h"
#include "tcpip_adapter.h"

static const char* TAG = "parse_handler";

extern const char *registers_temp_file;
extern const char *registers_file;
extern const char *timestamp_file;
extern int registers_size;
extern simple_structure *registers_data;
extern char timestamp[256];

#define DATA_INIT_BRACKET   0
#define ACCESS_INIT_BRACKET 1
#define ARRAY_INIT_BRACKET  2
#define READ_STRUCT_INIT    3
#define READ_STRUCT_END     4
#define ARRAY_END_BRACKET   5
#define DATA_END_BRACKET    6

static void parse_line(char* line, card_structure* data){
    unsigned int index = 0;
    unsigned int bytes = 0;
    char key_str[30] = "";
    while(sscanf(line+bytes, " \"%[^,\"]\":%n", key_str, &index) > 0){
        bytes += index;
        if(strcmp(key_str, "index") == 0){
            uint32_t value = 0;
            sscanf(line+bytes, " %u%n", &value, &index);
        }
        else if(strcmp(key_str, "cardType") == 0){
            uint8_t value = 0;
            sscanf(line+bytes, " %3" SCNu8 "%n", &value, &index);
            data->cardType = value;
        }
        else if(strcmp(key_str, "cardCode") == 0){
            uint32_t value1 = 0;
            uint32_t value2 = 0;
            sscanf(line+bytes, " [%u,%u]%n", &value1, &value2, &index);
            data->code1 = value1;
            data->code2 = value2;
        }
        else if(strcmp(key_str, "permisos") == 0){
            uint8_t value = 0;
            sscanf(line+bytes, " %3" SCNu8 "%n", &value, &index);
            data->permisos = value;
        }
        else if(strcmp(key_str, "idCanalHorario") == 0){
            uint16_t value = 0;
            char value_str[30] = "";
            sscanf(line+bytes, " %[^,]%n", value_str, &index);
            value = strcmp(value_str, "null") == 0 ? 0 : (uint16_t)atoi(value_str);
            data->canalHorario = value;
        }
        else if(strcmp(key_str, "antipassbackStatus") == 0){
            uint8_t value = 0;
            sscanf(line+bytes, " %3" SCNu8 "%n", &value, &index);
            data->antipassbackStatus = value;
        }
        else if(strcmp(key_str, "currentTimestamp") == 0){
            sscanf(line+bytes, " %[^,]", timestamp);
        }
        bytes += index;
        if(strlen(line+bytes) == 0)
            break;
        else
            ++bytes;
    }
}

static void parse_insert_card(FILE **f, card_structure *prueba, long int *write_pointer){
    long int file_pos = 0;
    if(strcmp(timestamp, "0") == 0){
        file_pos = ftell(*f);                                   // Save current pointer position
        fseek(*f, *write_pointer, SEEK_SET);                    // Restore registers pointer position
        fwrite(prueba, sizeof(card_structure), 1, *f);           // Write registers
        *write_pointer += sizeof(card_structure);                // Move registers pointer
        fseek(*f, file_pos, SEEK_SET);                          // Restore pointer position
        registers_size++;
    }
    else{
        for(int i = 0; i < registers_size; i++){
            if( prueba->cardType == registers_data[i].cardType
                && prueba->code1 == registers_data[i].code1
                && prueba->code2 == registers_data[i].code2){
                return;
            }
        }
        FILE *g = fopen(REG_FILE, "a");
        fseek(g, 0, SEEK_END);
        fwrite(prueba, sizeof(card_structure), 1, g);
        fclose(g);
    }
}

void parse_data(void){
    char line[200] = "";
    char tmp = 0;
    bool reg_tmp = 0;
    uint8_t state = 0;
    long int write_pointer = 0;
    int bytes = 0;
    struct timeval now;
    float current_time;
    card_structure prueba = {0};

    ESP_LOGI(TAG, "Reading file");

    FILE *f;
    if(strcmp(timestamp, "0") == 0){
        ESP_LOGI(TAG, "Registers file");
        f = fopen(REG_FILE, "r+");
        reg_tmp = 0;
    }
    else{
        ESP_LOGI(TAG, "Temporal file");
        f = fopen(REG_TMP_FILE, "r+");
        reg_tmp = 1;
    }

    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    gettimeofday(&now, NULL);
    current_time = now.tv_sec + now.tv_usec/1000000.0;

    while((tmp = fgetc(f)) != EOF){
        if(state == DATA_INIT_BRACKET && tmp == '{'){
            fscanf(f, " \"%[^\"]\"%n", line, &bytes);             // Scan for next string
            if(strcmp(line, "data") == 0)
                state = ACCESS_INIT_BRACKET;                        // Go to next state
        }
        else if(state == ACCESS_INIT_BRACKET && tmp == '{'){
            fscanf(f, " \"%[^\"]\"%n", line, &bytes);             // Scan for next string
            if(strcmp(line, "accessRecords") == 0)
                state = ARRAY_INIT_BRACKET;                         // Go to next state
        }
        else if(state == ARRAY_INIT_BRACKET && tmp == '[')
            state = READ_STRUCT_INIT;                               // Go to read registers
        else if(state == READ_STRUCT_INIT && tmp == '{'){
            ESP_LOGI(TAG, "Write pointer = %ld", write_pointer/sizeof(card_structure));
            fscanf(f, " %[^}]%n", line, &bytes);                    // Scan for next register
            parse_line(line, &prueba);                              // Parse register from line
            parse_insert_card(&f, &prueba, &write_pointer);
            state = READ_STRUCT_END;                                // Go to next state
        }
        else if(state == READ_STRUCT_END && tmp == '}')
            state = READ_STRUCT_INIT;                               // Go to read next register
        else if(state == READ_STRUCT_INIT && tmp == ']')
            state = ARRAY_END_BRACKET;                              // End of registers
        else if(state == ARRAY_END_BRACKET){
            fscanf(f, " %[^}]%n", line, &bytes);                    // Scan for timestamp string
            parse_line(line, NULL);
            ESP_LOGI(TAG, " - %s - ", timestamp);                        // Print timestamp

            FILE *g = fopen(REG_TIMESTAMP, "w");
            fprintf(g, "%s %d", timestamp, registers_size);
            fclose(g);
            state = DATA_END_BRACKET;                               // Go to read status
        }
        else if(state == DATA_END_BRACKET && tmp == '}'){
            fscanf(f, " ,%[^}]%n", line, &bytes);                   // Scan for status string
            ESP_LOGI(TAG, " - %s - ", line);                        // Print status
            break;
        }
    }
    ESP_LOGI(TAG, "Closing file");
    fclose(f);


    f = fopen(REG_FILE, "r");
    registers_data = (simple_structure*) realloc(registers_data, sizeof(simple_structure) * (registers_size) );
    for(int i = 0; i < registers_size; i++){
        fread(&prueba, sizeof(card_structure), 1, f);
        registers_data[i].cardType = prueba.cardType;
        registers_data[i].code1 = prueba.code1;
        registers_data[i].code2 = prueba.code2;
    }
    fclose(f);

    gettimeofday(&now, NULL);
    current_time = now.tv_sec + now.tv_usec/1000000.0 - current_time;
    printf("Elapsed time = %f\n", current_time);

    if(reg_tmp == 1)
        remove(REG_TMP_FILE);
    else
        truncate(REG_FILE, write_pointer);
}