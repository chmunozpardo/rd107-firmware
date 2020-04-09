#include "handler_wifi.h"

#define GOT_IPV4_BIT BIT(0)
#define CONNECTED_BITS (GOT_IPV4_BIT)

static const char *TAG = "wifi_handler";

static EventGroupHandle_t s_connect_event_group;

static uint16_t ap_count = 0;
static uint16_t number   = DEFAULT_SCAN_LIST_SIZE;

static wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];

ip4_str ip_addr = {0};
ip4_str gw_addr = {0};
uint8_t mac[6]  = {0};

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
    uint8_t i  = 0;
    int8_t out = 0;
    int8_t state;
    char in_opt[3] = "";
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
            if (ch == '\n')
            {
                break;
            }
            else if(ch == '\b')
            {
                if(i > 0 && i <= 3)
                {
                    in_opt[--i] = 0;
                }
            }
            else
            {
                if(i < 3) in_opt[i] = ch;
                ++i;
            }
        }
    }
    uint8_t max_i = (i < 50) ? i : 50;
    if(max_i == 0) state = 1;
    for(int j = 0; j < max_i; j++)
    {
        if(!isdigit(in_opt[j]))
        {
            ESP_LOGE(TAG, "Not a numeric option");
            out = -1;
            break;
        }
    }
    if(out != -1) out = atoi(in_opt);
    return out;
}

static int8_t enter_password(char *in_opt)
{
    uint8_t i;
    int8_t out = 0;
    uint8_t state;
    i = 0;
    state = 0;
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
            if (ch == '\n')
            {
                break;
            }
            else if(ch == '\b')
            {
                if(i > 0 && i <= 50) in_opt[--i] = 0;
            }
            else
            {
                if(i < 50) in_opt[i] = ch;
                ++i;
            }
        }
    }
    uint8_t max_i = (i < 50) ? i : 50;
    if(max_i == 0) state = 1;
    for(int j = 0; j < max_i; j++)
    {
        if(!isalnum(in_opt[j]))
        {
            ESP_LOGE(TAG, "Not an alphanumeric password");
            out = -1;
            break;
        }
    }
    return out;
}

void wifi_init(void)
{

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    memset(ap_info, 0, sizeof(ap_info));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);
    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        ESP_LOGI(TAG, "Network option %d: \t%s", i + 1, ap_info[i].ssid);
        //ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
        //ESP_LOGI(TAG, "Channel \t\t%d\n", ap_info[i].primary);
    }

    ESP_ERROR_CHECK(esp_wifi_stop());

    ESP_LOGI(TAG, "Select network from list [1-%d]. Enter 0 for default:", ap_count);
    int8_t opt = -1;
    char net_password[50] = "";
    while(opt == -1)
    {
        opt = enter_opt();
    }

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    s_connect_event_group = xEventGroupCreate();

    if(opt > 0){
        ESP_LOGI(TAG, "Network selected: %s", ap_info[opt-1].ssid);
        ESP_LOGI(TAG, "Enter password:");
        int8_t pass_out = -1;
        while(pass_out == -1)
        {
            memset(net_password, 0, 50);
            pass_out = enter_password(&net_password);
        }
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
        wifi_config_t wifi_config = {
            .sta = {
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