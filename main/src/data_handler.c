#include <string.h>
#include <stdlib.h>
#include "esp_http_client.h"
#include "esp_wifi.h"
#include "esp_eth.h"
#include "esp_log.h"

#include "connect.h"
#include "data_handler.h"
#include "parse_handler.h"

extern card_structure *registers_data;
extern int registers_size;
extern char timestamp[100];

static const char *TAG             = "data_handler";
static const char *TAG_HTTPS       = "https_handler";
static const char *url             = "http://192.168.10.241/control_acceso/obtenerMediosAccesoControlador";
static const char *id_controlador  = "16";
static const char *lastTimestamp   = timestamp;
static const char *database        = "GK2_Titanium";
static const char *api_token       = "dreamit-testing-rd107-2020";
static const char *nombreInstancia = "GK2_Titanium";

static esp_err_t _http_event_handle(esp_http_client_event_t *evt){
    switch(evt->event_id) {
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
            FILE *f;
            f = fopen(REG_FILE_JSON, "a");
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

void data_load(void){
    struct stat st;
    if (stat(REG_TIMESTAMP, &st) == 0) {
        FILE *f = fopen(REG_TIMESTAMP, "r");
        fscanf(f, "%s %d", timestamp, &registers_size);
        fclose(f);
        f = fopen(REG_FILE, "r");
        card_structure tempCard = {0};
        ESP_LOGI(TAG, "Loaded timestamp = %s | Total registers = %d", timestamp, registers_size);
        registers_data = (card_structure *) malloc(CARD_FULL_SIZE * registers_size);
        for(int i = 0; i < registers_size; i++){
            fread(&tempCard, CARD_FULL_SIZE, 1, f);
            registers_data[i].cardType = tempCard.cardType;
            registers_data[i].code1    = tempCard.code1;
            registers_data[i].code2    = tempCard.code2;
        }
        fclose(f);
    }
    else{
        strcpy(timestamp, "0");
        registers_size = 0;
        ESP_LOGI(TAG, "Data not loaded");
        data_request();
    }
}

void data_request(void){
    ESP_LOGI(TAG, "Last timestamp = %s | Total registers = %d", timestamp, registers_size);
    esp_http_client_config_t config = {
        .url            = url,
        .event_handler  = _http_event_handle,
        //.transport_type = HTTP_TRANSPORT_OVER_SSL,
        .buffer_size    = 16384,
        .buffer_size_tx = 16384,
        //.port           = 8080,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    char post_data[200] = "";

    strcat(post_data, "id_controlador=");
    strcat(post_data, id_controlador);
    strcat(post_data, "&lastTimestamp=");
    strcat(post_data, lastTimestamp);
    strcat(post_data, "&database=");
    strcat(post_data, database);
    strcat(post_data, "&api_token=");
    strcat(post_data, api_token);
    strcat(post_data, "&nombreInstancia=");
    strcat(post_data, nombreInstancia);
    strcat(post_data, "&binary=");
    strcat(post_data, "0");

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(
                TAG_HTTPS, "Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client)
                );
    }
    esp_http_client_cleanup(client);

    parse_data();
}