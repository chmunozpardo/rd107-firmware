#include "handler_wifi.h"

#define GOT_IPV4_BIT BIT(0)
#define CONNECTED_BITS (GOT_IPV4_BIT)

static const char *TAG = "wifi_handler";

static EventGroupHandle_t s_connect_event_group;
static ip4_addr_t s_ip_addr;

static uint16_t number = DEFAULT_SCAN_LIST_SIZE;
static wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
static uint16_t ap_count = 0;

static void on_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    memcpy(&s_ip_addr, &event->ip_info.ip, sizeof(s_ip_addr));
    xEventGroupSetBits(s_connect_event_group, GOT_IPV4_BIT);
}

static void on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
    ESP_ERROR_CHECK(esp_wifi_connect());
}

static void on_wifi_connect(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
}

static uint8_t enter_opt()
{
    uint8_t i;
    while(1)
    {
        i = 0;
        char in_opt[2] = "0";
        while(1)
        {
            char ch;
            ch = fgetc(stdin);
            if(ch != 0xFF)
            {
                fputc(ch, stdout);
                if (ch=='\n')
                {
                    break;
                }
                else
                {
                    in_opt[i] = ch;
                    ++i;
                    if(i >= 3) break;
                }
            }
        }
        if(i == 1 && isdigit(in_opt[0]))
        {
            i = atoi(in_opt);
            if(0 <= i && i < (ap_count+1)) break;
        }
        else if(i == 2 && isdigit(in_opt[0]) && isdigit(in_opt[1]))
        {
            i = atoi(in_opt);
            if(0 <= i && i < (ap_count+1)) break;
        }
        ESP_LOGI(TAG, "Try again");
    }
    return i;
}

static void enter_password(char *in_opt)
{
    uint8_t i;
    uint8_t state;
    while(1)
    {
        i = 0;
        state = 0;
        while(1)
        {
            char ch;
            ch = fgetc(stdin);
            if(ch != 0xFF)
            {
                fputc(ch, stdout);
                if (ch=='\n')
                {
                    break;
                }
                else
                {
                    in_opt[i] = ch;
                    ++i;
                    if(i >= 50) break;
                }
            }
        }
        for(int j = 0; j < i; j++)
        {
            if(!isalnum(in_opt[j]))
            {
                printf("Not alpha = %c\n", in_opt[j]);
                state = 1;
                break;
            }
        }
        if(state == 1)
        {
            ESP_LOGI(TAG, "Try again");
        }
        else
        {
            break;
        }
    }
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
    uint8_t opt;
    opt = enter_opt();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    s_connect_event_group = xEventGroupCreate();

    if(opt > 0){
        ESP_LOGI(TAG, "Network selected: %s", ap_info[opt-1].ssid);
        ESP_LOGI(TAG, "Enter password:");
        char net_password[50] = "";
        enter_password(&net_password);
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