#include "parse_handler.h"

static const char* TAG = "parse_handler";

static void parse_insert_card(CARD *input, uint32_t read_size)
{
    FILE *f           = NULL;
    bool check        =    0;
    uint32_t f_read   =    0;
    uint32_t pos_save =    0;
    for(int i = 0; i < read_size; i++)
    {
        xSemaphoreTake(reg_semaphore, portMAX_DELAY);
        f = fopen(REG_FILE, "r+");
        check = 0;
        while(check == 0)
        {
            pos_save = ftell(f);
            f_read   = fread(registers_data, CARD_FULL_SIZE, CARD_READER_SIZE, f);
            if(f_read == 0) break;
            for(int j = 0; j < f_read; j++)
            {
                if(input[i].index == registers_data[j].index)
                {
                    ESP_LOGI(TAG, "Modified old register");
                    fseek(f, j*CARD_FULL_SIZE, pos_save);
                    fwrite(&(input[i]), CARD_FULL_SIZE, 1, f);
                    check = 1;
                    break;
                }
            }
        }
        if(check == 0){
            ESP_LOGI(TAG, "Inserted new register");
            fwrite(&(input[i]), CARD_FULL_SIZE, 1, f);
            ++registers_size;
        }
        fclose(f);
        xSemaphoreGive(reg_semaphore);
    }
}

void parse_data(void)
{
    char line[100]      =   "";
    uint32_t count      =    0;
    uint32_t new_regs   =    0;
    uint32_t read_size  =    0;
    FILE *f             = NULL;
    FILE *g             = NULL;
    CARD *data_importer = NULL;

    f = fopen(REG_FILE_JSON, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    #ifdef DEBUG_INFO
        float current_time  =   0 ;
        struct timeval now  =  {0};
        gettimeofday(&now, NULL);
        current_time = now.tv_sec + now.tv_usec/1000000.0;
    #endif

    fscanf(f, " {\"data\":{\"countData\":%d,\"accessRecords\":", &new_regs);

    if(new_regs > 0)
    {
        ESP_LOGI(TAG, "New registers size = %d", new_regs);
        ESP_LOGI(TAG, "Importing registers...");
        if(timestamp > 0)
        {
            count         = new_regs / COPY_SIZE;
            data_importer = (CARD *) malloc(CARD_FULL_SIZE * COPY_SIZE);
            for(int i = 0; i < count; i++)
            {
                read_size  = fread (data_importer, CARD_FULL_SIZE, COPY_SIZE, f);
                parse_insert_card(data_importer, read_size);
            };
            read_size  = fread (data_importer, CARD_FULL_SIZE, new_regs%COPY_SIZE, f);
            parse_insert_card(data_importer, read_size);
            free(data_importer);
        }
        else
        {
            xSemaphoreTake(reg_semaphore, portMAX_DELAY);
            g     = fopen(REG_FILE, "a+");
            count = new_regs / CARD_READER_SIZE;
            for(int i = 0; i < count; i++)
            {
                read_size  = fread (registers_data, CARD_FULL_SIZE, CARD_READER_SIZE, f);
                registers_size += read_size;
                fwrite(registers_data, CARD_FULL_SIZE, read_size, g);
                RGB_SIGNAL(RGB_GREEN, (RGB_LEDS * i)/count, 0);
            };
            read_size  = fread (registers_data, CARD_FULL_SIZE, new_regs % CARD_READER_SIZE, f);
            registers_size += read_size;
            fwrite(registers_data, CARD_FULL_SIZE, read_size, g);
            RGB_SIGNAL(RGB_CYAN, RGB_LEDS, 0);
            fclose(g);
            xSemaphoreGive(reg_semaphore);
        }
        ESP_LOGI(TAG, "Finished importing registers");
    }
    else
    {
        ESP_LOGI(TAG, "No new registers");
    }

    fscanf(f, " ,\"currentTimestamp\":%llu},\"estado\":%[^}]", &timestamp, line);
    ESP_LOGI(TAG, "Timestamp  = %llu", timestamp);
    ESP_LOGI(TAG, "Status = %s", line);

    g = fopen(REG_TIMESTAMP, "w");
    fprintf(g, "%llu %u", timestamp, registers_size);
    fclose(g);
    fclose(f);
    remove(REG_FILE_JSON);

    #ifdef DEBUG_INFO
        gettimeofday(&now, NULL);
        current_time = now.tv_sec + now.tv_usec/1000000.0 - current_time;
        ESP_LOGI(TAG, "Elapsed time = %f", current_time);
    #endif
}