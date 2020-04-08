#include "handler_parse.h"

static const char* TAG = "parse_handler";

static DRAM_ATTR char line[10]      =   "";
static DRAM_ATTR uint32_t count     =    0;
static DRAM_ATTR uint32_t new_regs  =    0;
static DRAM_ATTR uint32_t read_size =    0;
static FILE *f = NULL;
static FILE *g = NULL;

static IRAM_ATTR void parse_insert_card(CARD *input, uint32_t read_size)
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

void IRAM_ATTR parse_command(void)
{
    f = fopen(REG_FILE_JSON, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    char cmd_reader[256] = "";
    char cmd_parser[100] = "";
    uint16_t num0 = 0;
    uint16_t num1 = 1;
    uint16_t num2 = 2;
    int chars = 0;
    int offset = 0;

    fscanf(f, " {\"estado\":\"OK\",\"data\":[%[^]\n]]}", cmd_reader);
    while(sscanf(cmd_reader + offset, " {\"id\":" "%3" SCNu16 ","
                        "\"id_controlador\":" "%3" SCNu16 ","
                        "\"id_usuario\":" "%3" SCNu16 ","
                        "\"comando\":\"%[^\"]\","
                        "\"estado\":\"En cola\"}%n", &num0, &num1, &num2, cmd_parser, &chars) > 0)
    {
        offset += chars;
        RELAY_SIGNAL(1);
        if(*(cmd_reader + offset) == '\0') break;
        else ++offset;
    }
    fclose(f);
    remove(REG_FILE_JSON);
}

void parse_register(void)
{
    f = fopen(REG_FILE_JSON, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    fscanf(f, " {\"apitoken\":\"%[^,\"]\",\"base_datos\":\"%[^,\"]\",\"idDevice\":%[^,\"],\"estado\":\"OK\"}",
        apitoken,
        database,
        idcontrolador
        );
    fclose(f);
    remove(REG_FILE_JSON);
}

void IRAM_ATTR parse_qr(void)
{
    f = fopen(REG_FILE_JSON, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    char qr_placeholder[6] = "";
    fscanf(f, " {\"estado\":\"OK\",\"data\":\"%6s\"}", qr_placeholder);
    ESP_LOGI(TAG, "QR Code = %s", qr_placeholder);
    if(strcmp(screen_qr, qr_placeholder) != 0)
    {
        strcpy(screen_qr, qr_placeholder);
        QR_SIGNAL();
    }
    fclose(f);
    remove(REG_FILE_JSON);
}

void IRAM_ATTR parse_data(void)
{
    f = fopen(REG_FILE_JSON, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    fscanf(f, " {\"data\":{\"countData\":%d,\"accessRecords\":", &new_regs);

    if(new_regs > 0)
    {
        ESP_LOGI(TAG, "New registers size = %d", new_regs);
        ESP_LOGI(TAG, "Importing registers...");
        if(timestamp > 0)
        {
            count = new_regs / COPY_SIZE;
            for(int i = 0; i < count; i++)
            {
                read_size  = fread (data_importer, CARD_FULL_SIZE, COPY_SIZE, f);
                parse_insert_card(data_importer, read_size);
            };
            read_size  = fread (data_importer, CARD_FULL_SIZE, new_regs%COPY_SIZE, f);
            parse_insert_card(data_importer, read_size);
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
        RGB_SIGNAL(RGB_CYAN, RGB_LEDS, 0);
        ESP_LOGI(TAG, "No new registers");
    }

    fscanf(f, " ,\"currentTimestamp\":%llu},\"estado\":%[^}]", &timestamp, line);
    ESP_LOGI(TAG, "Timestamp = %llu", timestamp);
    ESP_LOGI(TAG, "Status = %s", line);

    g = fopen(REG_TIMESTAMP, "w");
    fprintf(g, "%llu %u", timestamp, registers_size);
    fclose(g);
    fclose(f);
    remove(REG_FILE_JSON);
}