#include "handler_parse.h"
#include "handler_search.h"

static const char* TAG = "parse_handler";

static DRAM_ATTR char line[10]      = "";
static DRAM_ATTR uint32_t count     =  0;
static DRAM_ATTR uint32_t new_regs  =  0;
static DRAM_ATTR uint32_t read_size =  0;
static FILE *f = NULL;
static FILE *g = NULL;

void parse_reader(char *input)
{
    ESP_LOGD(TAG, "Input = %s", input);
    char rut_value[16] = "";
    if(sscanf(input, " https://portal.sidiv.registrocivil.cl/docstatus?RUN=%[^&]", rut_value) == 1)
    {
        search_rut(rut_value);
        return;
    }
    else if(sscanf(input, " %[^&]", rut_value) == 1)
    {
        search_reservation_code(rut_value);
        return;
    }
    SCREEN_SIGNAL("BAD", 1, DISPLAY_TIME);
    return;
}

void parse_register(void)
{
    f = fopen(FILE_JSON, "r");
    if(f == NULL)
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
    remove(FILE_JSON);
}

void parse_validation(void)
{
    f = fopen(FILE_JSON, "r");
    if(f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        f = fopen(FILE_JSON, "w");
        fprintf(f, "%s", "ERROR");
        fclose(f);
        return;
    }
    char validation_placeholder[6] = "";
    fscanf(f, " {\"estado\":\"%[^\",}]\"}", validation_placeholder);
    fclose(f);
    remove(FILE_JSON);
    f = fopen(FILE_JSON, "w");
    fprintf(f, "%s", validation_placeholder);
    fclose(f);
}

static IRAM_ATTR void parse_insert_card(CARD *input, uint32_t read_size)
{
    FILE *f           = NULL;
    bool check        =    0;
    uint32_t f_read   =    0;
    uint32_t pos_save =    0;
    for(int i = 0; i < read_size; i++)
    {
        xSemaphoreTake(card_semaphore, portMAX_DELAY);
        f = fopen(FILE_CARDS, "r+");
        if(f == NULL) f = fopen(FILE_CARDS, "w+");
        check = 0;
        while(check == 0)
        {
            pos_save = ftell(f);
            f_read   = fread(card_data, CARD_SIZE, CARD_READER_SIZE, f);
            if(f_read == 0) break;
            for(int j = 0; j < f_read; j++)
            {
                if(input[i].index == card_data[j].index)
                {
                    ESP_LOGD(TAG, "Modified old register");
                    fseek(f, j*CARD_SIZE, pos_save);
                    fwrite(&(input[i]), CARD_SIZE, 1, f);
                    check = 1;
                    break;
                }
            }
        }
        if(check == 0){
            ESP_LOGD(TAG, "Inserted new register");
            fwrite(&(input[i]), CARD_SIZE, 1, f);
            ++card_size;
        }
        fclose(f);
        xSemaphoreGive(card_semaphore);
    }
}

static IRAM_ATTR void parse_insert_reservation(RESERVATION *input, uint32_t read_size)
{
    FILE *f           = NULL;
    bool check        =    0;
    uint32_t f_read   =    0;
    uint32_t pos_save =    0;
    for(int i = 0; i < read_size; i++)
    {
        xSemaphoreTake(reservation_semaphore, portMAX_DELAY);
        f = fopen(FILE_RESERVATIONS, "r+");
        if(f == NULL) f = fopen(FILE_RESERVATIONS, "w+");
        check = 0;
        while(check == 0)
        {
            pos_save = ftell(f);
            f_read   = fread(reservation_data, RESERVATION_SIZE, RESERVATION_READER_SIZE, f);
            if(f_read == 0) break;
            for(int j = 0; j < f_read; j++)
            {
                if(input[i].index == card_data[j].index)
                {
                    ESP_LOGD(TAG, "Modified old reservation");
                    fseek(f, j*RESERVATION_SIZE, pos_save);
                    fwrite(&(input[i]), RESERVATION_SIZE, 1, f);
                    check = 1;
                    break;
                }
            }
        }
        if(check == 0){
            ESP_LOGD(TAG, "Inserted new reservation");
            fwrite(&(input[i]), RESERVATION_SIZE, 1, f);
            ++reservation_size;
        }
        fclose(f);
        xSemaphoreGive(reservation_semaphore);
    }
}

void IRAM_ATTR parse_command(void)
{
    f = fopen(FILE_JSON, "r");
    if(f == NULL)
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
        SCREEN_SIGNAL("GOOD", 1, DISPLAY_TIME);
        RELAY_SIGNAL(DISPLAY_TIME);
        if(*(cmd_reader + offset) == '\0') break;
        else ++offset;
    }
    fclose(f);
    remove(FILE_JSON);
}

void IRAM_ATTR parse_qr(void)
{
    f = fopen(FILE_JSON, "r");
    if(f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    char qr_placeholder[6] = "";
    fscanf(f, " {\"estado\":\"OK\",\"data\":\"%6s\"}", qr_placeholder);
    if(strcmp(screen_qr, qr_placeholder) != 0)
    {
        ESP_LOGD(TAG, "QR Code = %s", qr_placeholder);
        strcpy(screen_qr, qr_placeholder);
        SCREEN_SIGNAL(screen_qr, 0, 0);
    }
    fclose(f);
    remove(FILE_JSON);
}

void IRAM_ATTR parse_data(void)
{
    f = fopen(FILE_JSON, "r");
    if(f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    fscanf(f, " {\"data\":{\"countData\":%d,\"accessRecords\":", &new_regs);

    if(new_regs > 0)
    {
        ESP_LOGD(TAG, "New registers size = %d", new_regs);
        ESP_LOGD(TAG, "Importing registers...");
        if(timestamp > 0)
        {
            count = new_regs / COPY_SIZE;
            for(int i = 0; i < count; i++)
            {
                read_size  = fread(card_importer, CARD_SIZE, COPY_SIZE, f);
                parse_insert_card(card_importer, read_size);
            };
            read_size  = fread(card_importer, CARD_SIZE, new_regs%COPY_SIZE, f);
            parse_insert_card(card_importer, read_size);
        }
        else
        {
            xSemaphoreTake(card_semaphore, portMAX_DELAY);
            g     = fopen(FILE_CARDS, "a+");
            count = new_regs / CARD_READER_SIZE;
            for(int i = 0; i < count; i++)
            {
                read_size  = fread(card_data, CARD_SIZE, CARD_READER_SIZE, f);
                card_size += read_size;
                fwrite(card_data, CARD_SIZE, read_size, g);
                RGB_SIGNAL(RGB_GREEN, (RGB_LEDS * i)/count, 0);
            };
            read_size  = fread(card_data, CARD_SIZE, new_regs % CARD_READER_SIZE, f);
            card_size += read_size;
            fwrite(card_data, CARD_SIZE, read_size, g);
            RGB_SIGNAL(RGB_CYAN, RGB_LEDS, 0);
            fclose(g);
            xSemaphoreGive(card_semaphore);
        }
        ESP_LOGD(TAG, "Finished importing registers");
    }
    else
    {
        RGB_SIGNAL(RGB_CYAN, RGB_LEDS, 0);
        ESP_LOGD(TAG, "No new registers");
    }

    fscanf(f, " ,\"currentTimestamp\":%llu},\"estado\":%[^}]", &timestamp_temp, line);
    ESP_LOGD(TAG, "Timestamp = %llu", timestamp_temp);
    ESP_LOGD(TAG, "Status = %s", line);

    g = fopen(FILE_TIMESTAMP, "w");
    fprintf(g, "%llu %u %u", timestamp_temp, card_size, reservation_size);
    fclose(g);
    fclose(f);
    remove(FILE_JSON);
}

void IRAM_ATTR parse_reservations(void)
{
    f = fopen(FILE_JSON, "r");
    if(f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    fscanf(f, " {\"data\":{\"countData\":%d,\"accessRecords\":", &new_regs);

    if(new_regs > 0)
    {
        ESP_LOGD(TAG, "New reservations size = %d", new_regs);
        ESP_LOGD(TAG, "Importing reservations...");
        if(timestamp > 0)
        {
            count = new_regs / COPY_SIZE;
            for(int i = 0; i < count; i++)
            {
                read_size  = fread(reservation_importer, RESERVATION_SIZE, COPY_SIZE, f);
                parse_insert_reservation(reservation_importer, read_size);
            };
            read_size  = fread(reservation_importer, RESERVATION_SIZE, new_regs%COPY_SIZE, f);
            parse_insert_reservation(reservation_importer, read_size);
        }
        else
        {
            xSemaphoreTake(reservation_semaphore, portMAX_DELAY);
            g     = fopen(FILE_RESERVATIONS, "a+");
            count = new_regs / RESERVATION_READER_SIZE;
            for(int i = 0; i < count; i++)
            {
                read_size  = fread(reservation_data, RESERVATION_SIZE, RESERVATION_READER_SIZE, f);
                reservation_size += read_size;
                fwrite(reservation_data, RESERVATION_SIZE, read_size, g);
                RGB_SIGNAL(RGB_GREEN, (RGB_LEDS * i)/count, 0);
            };
            read_size  = fread(reservation_data, RESERVATION_SIZE, new_regs % RESERVATION_READER_SIZE, f);
            reservation_size += read_size;
            fwrite(reservation_data, RESERVATION_SIZE, read_size, g);
            RGB_SIGNAL(RGB_CYAN, RGB_LEDS, 0);
            fclose(g);
            xSemaphoreGive(reservation_semaphore);
        }
        ESP_LOGD(TAG, "Finished importing reservations");
    }
    else
    {
        RGB_SIGNAL(RGB_CYAN, RGB_LEDS, 0);
        ESP_LOGD(TAG, "No new reservations");
    }

    fscanf(f, " ,\"currentTimestamp\":%llu},\"estado\":%[^}]", &timestamp_temp, line);
    ESP_LOGD(TAG, "Timestamp = %llu", timestamp_temp);
    ESP_LOGD(TAG, "Status = %s", line);

    g = fopen(FILE_TIMESTAMP, "w");
    fprintf(g, "%llu %u %u", timestamp_temp, card_size, reservation_size);
    fclose(g);
    fclose(f);
    remove(FILE_JSON);
}