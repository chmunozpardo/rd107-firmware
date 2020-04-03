#include "handler_relay.h"

static const char* TAG = "relay_handler";

static uint8_t status = 0;

void relay_init(void)
{
    relay_task_queue = xQueueCreate(10, sizeof(uint8_t));
    gpio_pad_select_gpio(RELAY_GPIO);
    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(RELAY_GPIO, 0);
}

void relay_task(void *arg)
{
    while(1)
    {
        if(xQueueReceive(relay_task_queue, &status, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "Opening relay");
            gpio_set_level(RELAY_GPIO, 1);
            if(status != 0) vTaskDelay(status*1000/portTICK_PERIOD_MS);
            gpio_set_level(RELAY_GPIO, 0);
        }
    }
}