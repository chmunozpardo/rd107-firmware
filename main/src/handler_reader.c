#include "handler_reader.h"
#include "handler_parse.h"

static const char *TAG = "uart_events";

#define EX_UART_NUM UART_NUM_2
#define PATTERN_CHR_NUM    (3)

#define BUF_SIZE (256)
#define RD_BUF_SIZE (BUF_SIZE)

void reader_init(void)
{
    gpio_pad_select_gpio(WDI_PIN_TRIGGER);
    gpio_set_direction(WDI_PIN_TRIGGER, GPIO_MODE_OUTPUT);
    gpio_pullup_en(WDI_PIN_TRIGGER);
    gpio_set_level(WDI_PIN_TRIGGER, 1);

    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(EX_UART_NUM, &uart_config);
    uart_set_pin(EX_UART_NUM, WDI_PIN_TX, WDI_PIN_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &reader_task_queue, 0);
}

void reader_task(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    for(;;) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        gpio_set_level(WDI_PIN_TRIGGER, 0);
        if(xQueueReceive(reader_task_queue, (void * )&event, (portTickType)portMAX_DELAY))
        {
            bzero(dtmp, RD_BUF_SIZE);
            ESP_LOGD(TAG, "uart[%d] event:", EX_UART_NUM);
            switch(event.type) {
                case UART_DATA:
                    ESP_LOGD(TAG, "[UART DATA]: %d", event.size);
                    uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    if(event.size > 1)
                    {
                        ESP_LOGD(TAG, "[UART DATA]: %s", (char*)dtmp);
                        BUZZER_SIGNAL(1);
                        parse_reader((char*) dtmp);
                    }
                    break;
                case UART_FIFO_OVF:
                    ESP_LOGD(TAG, "hw fifo overflow");
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(reader_task_queue);
                    break;
                case UART_BUFFER_FULL:
                    ESP_LOGD(TAG, "ring buffer full");
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(reader_task_queue);
                    break;
                case UART_BREAK:
                    ESP_LOGD(TAG, "uart rx break");
                    break;
                case UART_PARITY_ERR:
                    ESP_LOGD(TAG, "uart parity error");
                    break;
                case UART_FRAME_ERR:
                    ESP_LOGD(TAG, "uart frame error");
                    break;
                default:
                    ESP_LOGD(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
        vTaskDelay(1000 /portTICK_PERIOD_MS);
        gpio_set_level(WDI_PIN_TRIGGER, 1);
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}