#include "handler_wifi.h"

#define GOT_IPV4_BIT BIT(0)
#define CONNECTED_BITS (GOT_IPV4_BIT)

static const char *TAG = "wifi_handler";

static EventGroupHandle_t s_connect_event_group;

static uint16_t ap_count = 0;
static uint16_t number   = DEFAULT_SCAN_LIST_SIZE;

static wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE] = {0};

ip4_str ip_addr = {0};
ip4_str gw_addr = {0};
uint8_t mac[6]  = {0};

static int8_t opt            = -1;
static uint8_t http_ind      =  0;
static char net_password[50] = "";

static esp_err_t config_get_handler(httpd_req_t *req)
{
    char ctm[100] = {'\0'};
    char lel[4] = {'\0'};

    strcpy(ctm, "<form action=\"/config\" method=\"post\">");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));
    for(uint8_t i = 0; i < ap_count; i++)
    {
        memset(ctm, '\0', 100);
        strcpy(ctm, "<input type=\"radio\" name=\"ssid\" value=\"");
        sprintf(lel, "%d", i+1);
        strcat(ctm, lel);
        strcat(ctm, "\">");
        httpd_resp_send_chunk(req, ctm, strlen(ctm));

        memset(ctm, '\0', 100);
        strcpy(ctm, "<label for=\"ssid_");
        sprintf(lel, "%d", i+1);
        strcat(ctm, lel);
        strcat(ctm, "\">");
        strcat(ctm, (char*)ap_info[i].ssid);
        strcat(ctm, "</label><br>");
        httpd_resp_send_chunk(req, ctm, strlen(ctm));
    }
    memset(ctm, '\0', 100);
    strcpy(ctm, "<label for=\"pass\">Wifi password:</label>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));
    memset(ctm, '\0', 100);
    strcpy(ctm, "<input type=\"text\" id=\"pass\" name=\"pass\"><br><br>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));
    memset(ctm, '\0', 100);
    strcpy(ctm, "<button type=\"submit\">Set wifi</button>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));
    memset(ctm, '\0', 100);
    strcpy(ctm, "</form>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));

    return ESP_OK;
}

static esp_err_t config_post_handler(httpd_req_t *req)
{
    char buf[100] = {'\0'};
    size_t buf_len = req->content_len;

    if (buf_len > 0)
    {
        memset(buf, '\0', buf_len+1);
        httpd_req_recv(req, buf, buf_len);
        ESP_LOGI(TAG, "POST query => %s", buf);
        char param[50] = {'\0'};
        if (httpd_query_key_value(buf, "ssid", param, sizeof(param)) == ESP_OK) opt = atoi(param);
        if (httpd_query_key_value(buf, "pass", param, sizeof(param)) == ESP_OK) strcpy(net_password, param);
        http_ind = 1;
    }
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t config_post =
    {
        .uri       = "/config",
        .method    = HTTP_POST,
        .handler   = config_post_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t config_get =
    {
        .uri       = "/config",
        .method    = HTTP_GET,
        .handler   = config_get_handler,
        .user_ctx  = NULL
    };

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &config_post);
        httpd_register_uri_handler(server, &config_get);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    httpd_stop(server);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

static void wifi_init_ap()
{

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    wifi_config_t wifi_config =
    {
        .ap =
        {
            .ssid = "rd107ap2020",
            .ssid_len = strlen("rd107ap2020"),
            .password = "dreamit1q2w3e",
            .max_connection = 2,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_ap finished. SSID:%s password:%s",
             "rd107ap2020", "dreamit1q2w3e");
}

static void wifi_end_ap()
{
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler));
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_LOGI(TAG, "wifi_end_ap finished");
}

static void on_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    memcpy(&ip_addr.ip_addr_i, &event->ip_info.ip, sizeof(ip4_addr_t));
    memcpy(&gw_addr.ip_addr_i, &event->ip_info.gw, sizeof(ip4_addr_t));
    xEventGroupSetBits(s_connect_event_group, GOT_IPV4_BIT);
}

static void on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
    ESP_ERROR_CHECK(esp_wifi_connect());
}

static int8_t enter_opt()
{
    uint8_t i = 0;
    char in_opt[3] = "";
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
            if(ch == '\b' && i > 0 && i <= 3) in_opt[--i] = 0;
            if(i < 3) in_opt[i++] = ch;
        }
    }
    uint8_t max_i = (i < 50) ? i : 50;
    for(int j = 0; j < max_i; j++)
    {
        if(!isdigit(in_opt[j]))
        {
            ESP_LOGE(TAG, "Not a numeric option");
            return -1;
        }
    }
    return atoi(in_opt);
}

static int8_t enter_password(char *in_opt)
{
    memset(in_opt, 0, 50);
    uint8_t i = 0;
    char ch;
    while(1)
    {
        ch = fgetc(stdin);
        if(ch != 0xFF)
        {
            fputc(ch, stdout);
            if(ch == '\n') break;
            if(ch == '\b') fprintf(stdout, "\033[K");
            if(ch == '\b' && i > 0 && i <= 50) in_opt[--i] = 0;
            if(i < 50) in_opt[i++] = ch;
        }
    }
    uint8_t max_i = (i < 50) ? i : 50;
    for(int j = 0; j < max_i; j++)
    {
        if(!isalnum(in_opt[j]))
        {
            ESP_LOGE(TAG, "Not an alphanumeric password");
            return -1;
        }
    }
    return 0;
}

void wifi_init(void)
{

    httpd_handle_t server = NULL;
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);
    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        ESP_LOGI(TAG, "Network option %d: \t%s", i + 1, ap_info[i].ssid);
        //ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
    }

    ESP_ERROR_CHECK(esp_wifi_stop());

    wifi_init_ap();
    server = start_webserver();

    ESP_LOGI(TAG, "Select network from list [1-%d]. Enter 0 for default:", ap_count);

    int8_t in_opt = -1;
    opt = -1;
    while(in_opt == -1 && http_ind != 1) in_opt = enter_opt();

    if(http_ind == 0) opt = in_opt;

    stop_webserver(server);
    wifi_end_ap();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    s_connect_event_group = xEventGroupCreate();

    if(opt > 0){
        ESP_LOGI(TAG, "Network selected: %s", ap_info[opt-1].ssid);
        if(http_ind == 0)
        {
            ESP_LOGI(TAG, "Enter password:");
            in_opt = -1;
            while(in_opt == -1) in_opt = enter_password(net_password);
        }
        ESP_LOGI(TAG, "Password used: %s", net_password);
        wifi_config_t wifi_config = {0};
        strcpy((char *)wifi_config.sta.ssid, (char *)ap_info[opt-1].ssid);
        strcpy((char *)wifi_config.sta.password, (char *)net_password);
        ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
    else
    {
        wifi_config_t wifi_config =
        {
            .sta =
            {
                .ssid = CONFIG_EXAMPLE_WIFI_SSID,
                .password = CONFIG_EXAMPLE_WIFI_PASSWORD,
            },
        };
        ESP_LOGI(TAG, "Using default network: %s", wifi_config.sta.ssid);
        ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
    xEventGroupWaitBits(s_connect_event_group, CONNECTED_BITS, true, true, portMAX_DELAY);
}