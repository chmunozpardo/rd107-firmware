#include "handler_web.h"
#include "handler_data.h"
#include "handler_parse.h"
#include "handler_search.h"
#include "handler_screen.h"

static const char *TAG            = "data_handler";
static const char *TAG_HTTPS      = "http_handler";

static esp_err_t err;

DRAM_ATTR char apitoken[30]     = {0};
DRAM_ATTR char database[20]     = {0};
DRAM_ATTR char idcontrolador[3] = {0};

static uint8_t http_ind =  0;

static httpd_handle_t server_code = NULL;

static DRAM_ATTR char post_data[256]    = "";
static DRAM_ATTR char lastTimestamp[20] = "0";

static DRAM_ATTR esp_http_client_handle_t client;

static void (*func_pointer)(void);

static esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id)
    {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG_HTTPS, "ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG_HTTPS, "CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG_HTTPS, "HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG_HTTPS, "HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG_HTTPS, "DATA, len=%d", evt->data_len);
            FILE *f;
            //printf("%.*s\n", evt->data_len, (char*)evt->data);
            f = fopen(FILE_JSON, "a+");
            if(f == NULL)
            {
                ESP_LOGE(TAG_HTTPS, "Could not append data to JSON file");
                break;
            }
            fwrite((char*)evt->data, 1, evt->data_len, f);
            fclose(f);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG_HTTPS, "FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG_HTTPS, "DISCONNECTED");
            break;
    }
    return ESP_OK;
}

static void data_client_set(void *func, const char *url)
{
    func_pointer = func;
    esp_http_client_config_t config =
    {
        .url            = url,
        .event_handler  = _http_event_handle,
        .buffer_size    = HTTPS_BUFFER,
        .buffer_size_tx = HTTPS_BUFFER,
    };
    client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    err = esp_http_client_perform(client);

    if(err == ESP_OK)
    {
        ESP_LOGD(
            TAG_HTTPS, "Status = %d, content_length = %d",
            esp_http_client_get_status_code(client),
            esp_http_client_get_content_length(client)
            );
        (*func_pointer)();
    }
    esp_http_client_cleanup(client);
}

static int8_t enter_opt(char *in_opt)
{
    memset(in_opt, 0, 1);
    uint8_t i = 0;
    char ch;
    while(1)
    {
        if(http_ind == 1) return -1;
        ch = fgetc(stdin);
        if(ch != 0xFF)
        {
            fputc(ch, stdout);
            if(ch == '\n') break;
            if(ch == '\b') fprintf(stdout, "\033[K");
            if(ch == '\b' && i > 0 && i <= 1) in_opt[--i] = 0;
            if(i < 1) in_opt[i++] = ch;
        }
    }
    uint8_t max_i = (i < 1) ? i : 1;
    for(int j = 0; j < max_i; j++)
    {
        if(in_opt[j] != 'y' && in_opt[j] != 'n')
        {
            ESP_LOGE(TAG, "Not a valid option");
            return -1;
        }
    }
    return 0;
}

static int8_t enter_registration_code(char *in_opt)
{
    memset(in_opt, 0, 7);
    uint8_t i = 0;
    char ch;
    while(1)
    {
        if(http_ind == 1) return -1;
        ch = fgetc(stdin);
        if(ch != 0xFF)
        {
            fputc(ch, stdout);
            if(ch == '\n') break;
            if(ch == '\b') fprintf(stdout, "\033[K");
            if(ch == '\b' && i > 0 && i <= 7) in_opt[--i] = 0;
            if(i < 7) in_opt[i++] = ch;
        }
    }
    uint8_t max_i = (i < 7) ? i : 7;
    for(int j = 0; j < max_i; j++)
    {
        if(!isdigit(in_opt[j]))
        {
            ESP_LOGE(TAG, "Not a numeric value");
            return -1;
        }
    }
    return 0;
}

static void register_on_api(char *registration_code)
{
    char reg_code[50] = "";
    if(http_ind == 0)
    {
        ESP_LOGI(TAG, "Enter the registration code:");
        int8_t in_opt = -1;

        while(in_opt == -1 && http_ind != 1) in_opt = enter_registration_code(reg_code);
    }
    stop_webserver(server_code);
    if(http_ind == 0) strcpy(registration_code, reg_code);

    ESP_LOGI(TAG, "Registration code used: %s", registration_code);

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
    strcat(post_data, registration_code);

    char tmp_str[24] = "";

    esp_read_mac(mac, 0);
    memset(tmp_str, 0, 24);
    sprintf(tmp_str,"%x:%x:%x:%x:%x:%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    strcat(post_data, "&mac=");
    strcat(post_data, tmp_str);

    memset(tmp_str, 0, 24);
    sprintf(tmp_str,"%u.%u.%u.%u", ip_addr.addr[0], ip_addr.addr[1], ip_addr.addr[2], ip_addr.addr[3]);
    strcat(post_data, "&ip=");
    strcat(post_data, tmp_str);

    memset(tmp_str, 0, 24);
    sprintf(tmp_str,"%u.%u.%u.%u", gw_addr.addr[0], gw_addr.addr[1], gw_addr.addr[2], gw_addr.addr[3]);
    strcat(post_data, "&gateway=");
    strcat(post_data, tmp_str);

    data_client_set(&parse_register, URL_REG);
    FILE *f  = fopen(FILE_CONFIG, "w");
    fprintf(f, "%s,%s,%s", apitoken, idcontrolador, database);
    fclose(f);
}

void data_register()
{
    struct stat st;
    int8_t opt = -1;
    char in_opt[1] = "";
    char opt_web   = 0;

    char registration_code[50] = "";

    reg_context_t server_context =
    {
        .registration_code = registration_code,
        .opt_web           = &opt_web,
        .http_ind          = &http_ind,
    };
    server_code = start_webserver(REG_WEBSERVER, &server_context);

    char screen_str[128] = "Connect to\n";
    char tmp_str[24]     = "";
    sprintf(tmp_str,"%u.%u.%u.%u", ip_addr.addr[0], ip_addr.addr[1], ip_addr.addr[2], ip_addr.addr[3]);
    strcat(screen_str, wifi_ssid);
    strcat(screen_str, "\nand browse to\n");
    strcat(screen_str, tmp_str);
    strcat(screen_str, "/config\nto configure this device");

    screen_print_conf(screen_str);

    if(stat(FILE_CONFIG, &st) == 0)
    {
        ESP_LOGI(TAG, "There is a previous device configuration. Do you want to load it? (y/n):");
        while(opt == -1 && http_ind != 1) opt = enter_opt(in_opt);
        if(http_ind == 1) in_opt[0] = opt_web;
        if(in_opt[0] == 'y')
        {
            stop_webserver(server_code);
            FILE *f  = fopen(FILE_CONFIG, "r");
            fscanf(f, "%[^,],%[^,],%[^,]", apitoken, idcontrolador, database);
            fclose(f);
            RGB_SIGNAL(RGB_CYAN, RGB_LEDS, 0);
        }
        else
        {
            register_on_api(registration_code);
        }
    }
    else
    {
        ESP_LOGI(TAG, "There is no previous device configuration.");
        register_on_api(registration_code);
    }
}

void IRAM_ATTR data_task(void *arg)
{
    struct stat st;
    if(stat(FILE_TIMESTAMP, &st) == 0)
    {
        FILE *f  = fopen(FILE_TIMESTAMP, "r");
        fscanf(f, "%llu %u %u", &timestamp_temp, &card_size, &reservation_size);
        fclose(f);
        ESP_LOGI(TAG, "Loaded data");
        RGB_SIGNAL(RGB_CYAN, RGB_LEDS, 0);
    }
    else
    {
        timestamp        = 0;
        card_size        = 0;
        reservation_size = 0;
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

        ESP_LOGI(TAG, "Last timestamp = %llu | Total registers = %u | Total reservations = %u", timestamp, card_size, reservation_size);

        data_client_set(&parse_data, URL);
        data_client_set(&parse_reservations, URL_RESERVATIONS);
        data_client_set(&parse_command, URL_COMMAND);
        data_client_set(&parse_qr, URL_QR);

        vTaskPrioritySet(wiegand_task_handle, 1);
        vTaskPrioritySet(rgb_task_handle    , 2);
        vTaskPrioritySet(relay_task_handle  , 2);
        vTaskPrioritySet(buzzer_task_handle , 2);
        vTaskPrioritySet(screen_task_handle     , 2);

        /*
        printf("Sizes = %u, %u\n", reservation_size, card_size);

        uint32_t read_size = 0;
        uint32_t count     = 0;
        uint32_t rem       = 0;

        FILE *f = NULL;

        count = card_size / CARD_READER_SIZE;
        rem   = card_size % CARD_READER_SIZE;
        f = fopen(FILE_CARDS, "r");
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
                printf("%u, %s, %s, %s, %s\n",reservation_data[j].index, reservation_data[j].code,reservation_data[j].qr, strftime_buf, strftime_buf_end);
            }
        }
        read_size  = fread (reservation_data, RESERVATION_SIZE, rem, f);
        for(int j = 0; j < read_size; j++)
        {
            strftime(strftime_buf, sizeof(strftime_buf), "%c", localtime((time_t*)&reservation_data[j].init_time));
            strftime(strftime_buf_end, sizeof(strftime_buf_end), "%c", localtime((time_t*)&reservation_data[j].end_time));
            printf("%u, %8s, %6s, %s, %s\n",reservation_data[j].index, reservation_data[j].code,reservation_data[j].qr, strftime_buf, strftime_buf_end);
        }
        fclose(f);*/

        timestamp = timestamp_temp;
        vTaskDelay(5000/portTICK_PERIOD_MS);
    }
}