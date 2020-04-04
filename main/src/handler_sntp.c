#include "handler_sntp.h"

static const char *TAG = "sntp_handler";

static char strftime_buf[64];

static time_t now         =  0 ;
static struct tm timeinfo = {0};
static int retry          =  0 ;

static void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "NTP synchronized");
}

void ntp_init(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    setenv("TZ", "<-03>3", 1);
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

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Chile is: %s", strftime_buf);
}