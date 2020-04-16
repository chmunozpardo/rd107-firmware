#include "handler_wifi.h"
#include "handler_web.h"

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
    char ctm[128] = {'\0'};
    char lel[4] = {'\0'};

    char chunk[1024];
    size_t read_bytes;

    int fd = open("/www/head.html", O_RDONLY, 0);
    if (fd == -1)
    {
        ESP_LOGE(TAG, "Failed to open file : /www/head.html");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/html");

    do {
        read_bytes = read(fd, chunk, 1024);
        if (read_bytes == -1)
        {
            ESP_LOGE(TAG, "Failed to read file : /www/head.html");
        }
        else if (read_bytes > 0)
        {
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK)
            {
                close(fd);
                ESP_LOGE(TAG, "File sending failed!");
                httpd_resp_sendstr_chunk(req, NULL);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    close(fd);

    strcpy(ctm, "<form action=/config method=post>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));
    memset(ctm, '\0', 128);
    strcpy(ctm, "<label for=opt>Select the Wifi network:</label><br><div class=gr-in>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));
    memset(ctm, '\0', 128);
    strcpy(ctm, "<div class=in-gr><input class=opt type=radio name=ssid value=0><label align=left for=ssid_0>Default</label></div>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));
    for(uint8_t i = 0; i < ap_count; i++)
    {
        memset(ctm, '\0', 128);
        strcpy(ctm, "<div class=in-gr><input class=opt type=radio name=ssid value=");
        sprintf(lel, "%d", i+1);
        strcat(ctm, lel);
        strcat(ctm, ">");
        httpd_resp_send_chunk(req, ctm, strlen(ctm));

        memset(ctm, '\0', 128);
        strcpy(ctm, "<label align=left for=ssid_");
        sprintf(lel, "%d", i+1);
        strcat(ctm, lel);
        strcat(ctm, ">");
        strcat(ctm, (char*)ap_info[i].ssid);
        strcat(ctm, "</label></div>");
        httpd_resp_send_chunk(req, ctm, strlen(ctm));
    }

    memset(ctm, '\0', 128);
    strcpy(ctm, "</div>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));

    memset(ctm, '\0', 128);
    strcpy(ctm, "<label for=pass>Enter Wifi password if select:</label><br><br>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));
    memset(ctm, '\0', 128);
    strcpy(ctm, "<input class=opt maxlength=50 type=text id=pass name=pass><button class=opt type=submit>Set wifi</button></form>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));

    fd = open("/www/end.html", O_RDONLY, 0);
    if (fd == -1)
    {
        ESP_LOGE(TAG, "Failed to open file : /www/end.html");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    do {
        read_bytes = read(fd, chunk, 1024);
        if (read_bytes == -1)
        {
            ESP_LOGE(TAG, "Failed to read file : /www/end.html");
        } else if (read_bytes > 0)
        {
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK)
            {
                close(fd);
                ESP_LOGE(TAG, "File sending failed!");
                httpd_resp_sendstr_chunk(req, NULL);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    close(fd);

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
    server = start_webserver(config_get_handler, config_post_handler);

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