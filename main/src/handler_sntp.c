#include "handler_sntp.h"

static const char *TAG = "sntp_handler";

time_t system_now = 0;

static void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "NTP synchronized");
    time(&system_now);
}

void ntp_init(void)
{
    int retry             =  0 ;
    char strftime_buf[32] = {0};
    struct tm timeinfo    = {0};

    ESP_LOGI(TAG, "Initializing SNTP");
    setenv("TZ", LOCAL_TIMEZONE, 1);
    tzset();
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "ntp.shoa.cl");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    sntp_init();

    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < 10) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, 10);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    localtime_r(&system_now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Chile is: %s", strftime_buf);
}