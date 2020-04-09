#include "handler_data.h"
#include "handler_parse.h"

static esp_err_t _http_event_handle(esp_http_client_event_t *evt);

static const char *TAG            = "data_handler";
static const char *TAG_HTTPS      = "http_handler";

DRAM_ATTR char apitoken[30]     = {0};
DRAM_ATTR char database[20]     = {0};
DRAM_ATTR char idcontrolador[3] = {0};

static DRAM_ATTR char post_data[256]    = "";
static DRAM_ATTR char lastTimestamp[20] = "0";

static DRAM_ATTR esp_http_client_handle_t client;
static DRAM_ATTR esp_http_client_config_t config =
    {
        .url            = URL,
        .event_handler  = _http_event_handle,
        .buffer_size    = HTTPS_BUFFER,
        .buffer_size_tx = HTTPS_BUFFER,
    };

static DRAM_ATTR esp_http_client_config_t config_reservations =
    {
        .url            = URL_RESERVATIONS,
        .event_handler  = _http_event_handle,
        .buffer_size    = HTTPS_BUFFER,
        .buffer_size_tx = HTTPS_BUFFER,
    };

static DRAM_ATTR esp_http_client_config_t config_cmd =
    {
        .url            = URL_COMMAND,
        .event_handler  = _http_event_handle,
        .buffer_size    = HTTPS_BUFFER,
        .buffer_size_tx = HTTPS_BUFFER,
    };

static DRAM_ATTR esp_http_client_config_t config_qr =
    {
        .url            = URL_QR,
        .event_handler  = _http_event_handle,
        .buffer_size    = HTTPS_BUFFER,
        .buffer_size_tx = HTTPS_BUFFER,
    };

static esp_err_t err;

static esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id)
    {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG_HTTPS, "ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG_HTTPS, "CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG_HTTPS, "HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG_HTTPS, "HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG_HTTPS, "DATA, len=%d", evt->data_len);
            printf("%.*s\n", evt->data_len, (char*)evt->data);
            FILE *f;
            f = fopen(REG_FILE_JSON, "a+");
            if (f == NULL)
                break;
            fwrite((char*)evt->data, 1, evt->data_len, f);
            fclose(f);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG_HTTPS, "FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG_HTTPS, "DISCONNECTED");
            break;
    }
    return ESP_OK;
}

void data_register()
{
    ESP_LOGI(TAG, "Enter the registration code:");
    uint8_t i;
    uint8_t state = 0;
    char code[7] = "";
    while(1)
    {
        i = 0;
        state = 0;
        memset(code, 0, 7);
        while(1)
        {
            char ch;
            ch = fgetc(stdin);
            if(ch != 0xFF)
            {
                fputc(ch, stdout);
                if(ch == '\b'){
                    fprintf(stdout, "\033[K");
                }
                if (ch=='\n')
                {
                    break;
                }
                else if(ch=='\b')
                {
                    if(i > 0)
                    {
                        code[--i] = 0;
                    }
                }
                else
                {
                    code[i++] = ch;
                    if(i > 6)
                    {
                        fputc('\n', stdout);
                        break;
                    }
                }
            }
        }
        if(i > 0)
        {
            for(int j = 0; j < i; j++)
            {
                if(!isdigit(code[j]))
                {
                    printf("Not digit = %c\n", code[j]);
                    state = 1;
                    break;
                }
            }
        }
        else
        {
            state = 1;
        }
        if(state == 1)
        {
            ESP_LOGI(TAG, "Try again please.");
        }
        else
        {
            break;
        }
    }

    memset(post_data, 0, 200);
    strcat(post_data, "modelo="\
                      RD_MODELO\
                      "&version="\
                      RD_VERSION\
                      "&serie="\
                      RD_SERIE\
                      "&cantidadCanales="\
                      RD_CANALES\
                      "&code=");
    strcat(post_data, code);

    char tmp_str[24] = "";

    esp_read_mac(mac, 0);
    memset(tmp_str, 0, 24);
    sprintf(tmp_str,"%x:%x:%x:%x:%x:%x",
            mac[0],
            mac[1],
            mac[2],
            mac[3],
            mac[4],
            mac[5]);
    strcat(post_data, "&mac=");
    strcat(post_data, tmp_str);

    memset(tmp_str, 0, 24);
    sprintf(tmp_str,"%u.%u.%u.%u",
            ip_addr.addr[0],
            ip_addr.addr[1],
            ip_addr.addr[2],
            ip_addr.addr[3]);
    strcat(post_data, "&ip=");
    strcat(post_data, tmp_str);

    memset(tmp_str, 0, 24);
    sprintf(tmp_str,"%u.%u.%u.%u",
            gw_addr.addr[0],
            gw_addr.addr[1],
            gw_addr.addr[2],
            gw_addr.addr[3]);
    strcat(post_data, "&gateway=");
    strcat(post_data, tmp_str);

    esp_http_client_config_t config_reg =
    {
        .url            = URL_REG,
        .event_handler  = _http_event_handle,
        .buffer_size    = HTTPS_BUFFER,
        .buffer_size_tx = HTTPS_BUFFER,
    };
    client = esp_http_client_init(&config_reg);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        ESP_LOGI(
            TAG_HTTPS, "Status = %d, content_length = %d",
            esp_http_client_get_status_code(client),
            esp_http_client_get_content_length(client)
            );
        parse_register();
    }
    esp_http_client_cleanup(client);
}

void IRAM_ATTR data_task(void *arg)
{
    struct stat st;
    bool start_up = 0;
    strcpy(apitoken, "dreamit-testing-rd107-2020");
    strcpy(idcontrolador, "1");
    strcpy(database, "GK2_Industrias");
    if (stat(REG_TIMESTAMP, &st) == 0)
    {
        start_up = 1;

        FILE *f  = fopen(REG_TIMESTAMP, "r");
        fscanf(f, "%llu %u", &timestamp, &registers_size);
        fclose(f);

        ESP_LOGI(TAG, "Loaded data");
        RGB_SIGNAL(RGB_CYAN, RGB_LEDS, 0);
        vTaskPrioritySet(wiegand_task_handle, 1);
        vTaskPrioritySet(rgb_task_handle    , 2);
        vTaskPrioritySet(relay_task_handle  , 2);
        vTaskPrioritySet(buzzer_task_handle , 2);
        vTaskPrioritySet(qr_task_handle     , 2);
    }
    else
    {
        start_up       = 0;
        timestamp      = 0;
        registers_size = 0;
        ESP_LOGI(TAG, "Empty local data");
    }

    while(1)
    {

        memset(post_data, 0, 200);
        strcat(post_data, "api_token=");
        strcat(post_data, apitoken);
        strcat(post_data, "&database=");
        strcat(post_data, database);
        strcat(post_data, "&id_controlador=");
        strcat(post_data, idcontrolador);
        strcat(post_data, "&nombreInstancia=");
        strcat(post_data, database);
        strcat(post_data, "&lastTimestamp=");
        sprintf(lastTimestamp, "%llu", timestamp);
        strcat(post_data, lastTimestamp);
        strcat(post_data, "&controladorTimestamp=");
        sprintf(lastTimestamp, "%ld", system_now);
        strcat(post_data, lastTimestamp);

        ESP_LOGI(TAG, "Last timestamp = %llu | Total registers = %u", timestamp, registers_size);

        client = esp_http_client_init(&config);
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_post_field(client, post_data, strlen(post_data));

        err = esp_http_client_perform(client);

        if (err == ESP_OK)
        {
            ESP_LOGI(
                TAG_HTTPS, "Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client)
                );
            parse_data();
            if(start_up == 0)
            {
                start_up = 1;
                vTaskPrioritySet(wiegand_task_handle, 1);
                vTaskPrioritySet(rgb_task_handle    , 2);
                vTaskPrioritySet(relay_task_handle  , 2);
                vTaskPrioritySet(buzzer_task_handle , 2);
                vTaskPrioritySet(qr_task_handle     , 2);
            }
        }

        esp_http_client_cleanup(client);

        client = esp_http_client_init(&config_reservations);
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_post_field(client, post_data, strlen(post_data));

        err = esp_http_client_perform(client);

        if (err == ESP_OK)
        {
            ESP_LOGI(
                TAG_HTTPS, "Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client)
                );
            parse_reservations();
        }
        esp_http_client_cleanup(client);

        client = esp_http_client_init(&config_cmd);
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_post_field(client, post_data, strlen(post_data));

        err = esp_http_client_perform(client);

        if (err == ESP_OK)
        {
            ESP_LOGI(
                TAG_HTTPS, "Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client)
                );
            parse_command();
        }
        esp_http_client_cleanup(client);

        client = esp_http_client_init(&config_qr);
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_post_field(client, post_data, strlen(post_data));

        err = esp_http_client_perform(client);

        if (err == ESP_OK)
        {
            ESP_LOGI(
                TAG_HTTPS, "Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client)
                );
            parse_qr();
        }
        esp_http_client_cleanup(client);

        /*printf("Sizes = %u, %u\n", reservation_size, registers_size);

        uint32_t read_size = 0;
        uint32_t count     = 0;
        uint32_t rem       = 0;

        FILE *f = NULL;

        count = registers_size / CARD_READER_SIZE;
        rem   = registers_size % CARD_READER_SIZE;
        f = fopen(REG_FILE, "r");
        for(int i = 0; i < count; i++)
        {
            read_size  = fread (card_data, CARD_SIZE, CARD_READER_SIZE, f);
            for(int j = 0; j < read_size; j++)
            {
                printf("%u, %u, %u\n",card_data[j].index, card_data[j].code1, card_data[j].code2);
            }
        }
        read_size  = fread (card_data, CARD_SIZE, rem, f);
        for(int j = 0; j < read_size; j++)
        {
            printf("%u, %u, %u\n",card_data[j].index, card_data[j].code1, card_data[j].code2);
        }
        fclose(f);

        count = reservation_size / RESERVATION_READER_SIZE;
        rem   = reservation_size % RESERVATION_READER_SIZE;
        f = fopen(FILE_RESERVATIONS, "r");
        for(int i = 0; i < count; i++)
        {
            read_size  = fread (reservation_data, RESERVATION_SIZE, RESERVATION_READER_SIZE, f);
            for(int j = 0; j < read_size; j++)
            {
                strftime(strftime_buf, sizeof(strftime_buf), "%c", localtime((time_t*)&reservation_data[j].init_time));
                strftime(strftime_buf_end, sizeof(strftime_buf_end), "%c", localtime((time_t*)&reservation_data[j].end_time));
                printf("%u, %s, %s\n",reservation_data[j].index, strftime_buf, strftime_buf_end);
            }
        }
        read_size  = fread (reservation_data, RESERVATION_SIZE, rem, f);
        for(int j = 0; j < read_size; j++)
        {
            strftime(strftime_buf, sizeof(strftime_buf), "%c", localtime((time_t*)&reservation_data[j].init_time));
            strftime(strftime_buf_end, sizeof(strftime_buf_end), "%c", localtime((time_t*)&reservation_data[j].end_time));
            printf("%u, %s, %s\n",reservation_data[j].index, strftime_buf, strftime_buf_end);
        }
        fclose(f);*/

        timestamp = timestamp_temp;

        vTaskDelay(5000/portTICK_PERIOD_MS);
    }
}