#include "connect.h"
#include "data_handler.h"
#include "parse_handler.h"

static const char *TAG       = "data_handler";
static const char *TAG_HTTPS = "https_handler";

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

void data_request(void *arg)
{
    struct stat st;
    if (stat(REG_TIMESTAMP, &st) == 0)
    {
        FILE *f = fopen(REG_TIMESTAMP, "r");
        fscanf(f, "%llu %u", &timestamp, &registers_size);
        fclose(f);
        ESP_LOGI(TAG, "Loaded data");
        RGB_SIGNAL(RGB_CYAN, RGB_LEDS, 0);
    }
    else
    {
        timestamp      = 0;
        registers_size = 0;
        ESP_LOGI(TAG, "Empty local data");
    }

    while(1)
    {
        ESP_LOGI(TAG, "Last timestamp = %llu | Total registers = %u", timestamp, registers_size);
        esp_http_client_config_t config =
        {
            .url            = URL,
            .event_handler  = _http_event_handle,
            //.transport_type = HTTP_TRANSPORT_OVER_SSL,
            .buffer_size    = 16384,
            .buffer_size_tx = 16384,
            //.port           = 8080,
        };

        esp_http_client_handle_t client = esp_http_client_init(&config);

        char post_data[200]     = "";
        char lastTimestamp[100] = "0";

        strcat(post_data, "id_controlador=");
        strcat(post_data, ID_CONTROLADOR);
        strcat(post_data, "&database=");
        strcat(post_data, DATABASE);
        strcat(post_data, "&api_token=");
        strcat(post_data, API_TOKEN);
        strcat(post_data, "&nombreInstancia=");
        strcat(post_data, NOMBRE_INSTANCIA);
        strcat(post_data, "&lastTimestamp=");
        sprintf(lastTimestamp, "%llu", timestamp);
        strcat(post_data, lastTimestamp);

        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_post_field(client, post_data, strlen(post_data));

        esp_err_t err = esp_http_client_perform(client);

        if (err == ESP_OK)
        {
            ESP_LOGI(
                TAG_HTTPS, "Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client)
                );
            esp_http_client_cleanup(client);
            parse_data();
        }

        vTaskDelay(5000/portTICK_PERIOD_MS);
    }
}