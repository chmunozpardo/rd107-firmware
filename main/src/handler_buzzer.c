#include "handler_buzzer.h"

static const char* TAG = "buzzer_handler";

static uint8_t status = 0;

void buzzer_init(void)
{
    buzzer_task_queue = xQueueCreate(10, sizeof(uint8_t));
    gpio_pad_select_gpio(BUZZER_GPIO);
    gpio_set_direction(BUZZER_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BUZZER_GPIO, 0);
}

void buzzer_task(void *arg)
{
    while(1)
    {
        if(xQueueReceive(buzzer_task_queue, &status, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "MAKE SOME NOISEEEE!!!");
            gpio_set_level(BUZZER_GPIO, 1);
            if(status != 0) vTaskDelay(status*1000/portTICK_PERIOD_MS);
            gpio_set_level(BUZZER_GPIO, 0);
        }
    }
}