#include "handler_files.h"

static const char *TAG = "fs_handler";

void fs_init(void)
{
    esp_err_t ret;
    size_t total = 0, used = 0;
    ESP_LOGI(TAG, "Mounting SPIFFS partition");
    esp_vfs_spiffs_conf_t conf_spiffs =
    {
        .base_path = "/spiffs",
        .partition_label = "storage",
        .max_files = 5,
        .format_if_mount_failed = true
    };

    ret = esp_vfs_spiffs_register(&conf_spiffs);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL) ESP_LOGE(TAG, "Failed to mount or format filesystem");
        else if (ret == ESP_ERR_NOT_FOUND) ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        else ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        return;
    }
    
    ret = esp_spiffs_info("storage", &total, &used);
    if (ret != ESP_OK) ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    else ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);

    ESP_LOGI(TAG, "Mounting www partition");
    esp_vfs_spiffs_conf_t conf_www =
    {
        .base_path = "/www",
        .partition_label = "www",
        .max_files = 5,
        .format_if_mount_failed = true
    };

    ret = esp_vfs_spiffs_register(&conf_www);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL) ESP_LOGE(TAG, "Failed to mount or format filesystem");
        else if (ret == ESP_ERR_NOT_FOUND) ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        else ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        return;
    }
    
    ret = esp_spiffs_info("www", &total, &used);
    if (ret != ESP_OK) ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    else ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
}