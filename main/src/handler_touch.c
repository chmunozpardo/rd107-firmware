#include "handler_spi.h"
#include "handler_touch.h"
#include "handler_screen.h"

static DRAM_ATTR xQueueHandle debounce_queue = NULL;

static TP_DEV sTP_DEV;
static TP_DRAW sTP_Draw;

static DRAM_ATTR uint8_t debounce_touch = 0;

static char keyboard_input_str[15] = "";

static uint8_t keyboard_pos = 0;

static IRAM_ATTR void debounce_isr(void* arg)
{
    BaseType_t xTaskWokenByReceive = pdFALSE;
    uint8_t debounce_touch_isr = gpio_get_level(LCD_PIN_IRQ);
    if(debounce_touch_isr == 0) xQueueSendFromISR(debounce_queue, &debounce_touch_isr, &xTaskWokenByReceive);
    if(xTaskWokenByReceive != pdFALSE) portYIELD_FROM_ISR();
}

static uint16_t touch_read_adc(uint8_t cmd)
{
    uint16_t data = 0;
    data = screen_read_byte(cmd);
    return data;
}

static void touch_read_adc_average(uint8_t cmd, uint16_t *mean_ptr, float *std_ptr)
{
    float std                 =  0;
    float std_tmp             =  0;
    uint8_t i                 =  0;
    uint16_t mean             =  0;
    uint16_t buf[TOUCH_READ_N]  = {0};

    for (i = 0; i < TOUCH_READ_N; i++)
    {
        buf[i] = touch_read_adc(cmd);
        mean  += buf[i];
    }

    *mean_ptr = mean / TOUCH_READ_N;

    for (i = 0; i < TOUCH_READ_N; i++)
    {
        std_tmp = (float)buf[i] - (float)*mean_ptr;
        std    += std_tmp * std_tmp;
    }
    *std_ptr = sqrt(std / TOUCH_READ_N);
}

static bool touch_read_xy(uint16_t *x_ch_mean, uint16_t *y_ch_mean)
{
    float x_std, y_std;
    uint16_t x_mean, y_mean;

    touch_read_adc_average(0xDC, &x_mean, &x_std);
    touch_read_adc_average(0x9C, &y_mean, &y_std);

    if(x_std < TOUCH_STD_RANGE && y_std < TOUCH_STD_RANGE)
    {
        *x_ch_mean = x_mean;
        *y_ch_mean = y_mean;
        return true;
    }
    return false;
}

static unsigned char touch_input()
{
    if(!gpio_get_level(LCD_PIN_IRQ))
    {
        if(touch_read_xy(&sTP_DEV.Xpoint, &sTP_DEV.Ypoint))
        {
            if(sTP_DEV.TP_Scan_Dir == R2L_D2U)
            {
                sTP_Draw.Xpoint = sTP_DEV.fXfac * sTP_DEV.Xpoint +
                                  sTP_DEV.iXoff;
                sTP_Draw.Ypoint = sTP_DEV.fYfac * sTP_DEV.Ypoint +
                                  sTP_DEV.iYoff;
            }
            else if(sTP_DEV.TP_Scan_Dir == L2R_U2D)
            {
                sTP_Draw.Xpoint = sLCD_DIS.LCD_Dis_Column -
                                  sTP_DEV.fXfac * sTP_DEV.Xpoint -
                                  sTP_DEV.iXoff;
                sTP_Draw.Ypoint = sLCD_DIS.LCD_Dis_Page -
                                  sTP_DEV.fYfac * sTP_DEV.Ypoint -
                                  sTP_DEV.iYoff;
            }
            else if(sTP_DEV.TP_Scan_Dir == U2D_R2L)
            {
                sTP_Draw.Xpoint = sTP_DEV.fXfac * sTP_DEV.Ypoint +
                                  sTP_DEV.iXoff;
                sTP_Draw.Ypoint = sTP_DEV.fYfac * sTP_DEV.Xpoint +
                                  sTP_DEV.iYoff;
            }
            else
            {
                sTP_Draw.Xpoint = sLCD_DIS.LCD_Dis_Column -
                                  sTP_DEV.fXfac * sTP_DEV.Ypoint -
                                  sTP_DEV.iXoff;
                sTP_Draw.Ypoint = sLCD_DIS.LCD_Dis_Page -
                                  sTP_DEV.fYfac * sTP_DEV.Xpoint -
                                  sTP_DEV.iYoff;
            }
        }
        if(0 == (sTP_DEV.chStatus & TP_PRESS_DOWN))
        {
            sTP_DEV.chStatus = TP_PRESS_DOWN | TP_PRESSED;
            sTP_DEV.Xpoint0 = sTP_DEV.Xpoint;
            sTP_DEV.Ypoint0 = sTP_DEV.Ypoint;
        }
    }
    else
    {
        if(sTP_DEV.chStatus & TP_PRESS_DOWN)
        {
            sTP_DEV.chStatus &= ~(1 << 7);
        }
        else
        {
            sTP_DEV.Xpoint0 = 0;
            sTP_DEV.Ypoint0 = 0;
            sTP_DEV.Xpoint = 0xffff;
            sTP_DEV.Ypoint = 0xffff;
        }
    }
    return (sTP_DEV.chStatus & TP_PRESS_DOWN);
}

void touch_init_f(void)
{
    debounce_queue = xQueueCreate(10, sizeof(uint8_t) * 1);
    sTP_DEV.TP_Scan_Dir = SCAN_DIR_DFT;

    if(sTP_DEV.TP_Scan_Dir == D2U_L2R)
    {
        sTP_DEV.fXfac = -0.132443F;
        sTP_DEV.fYfac = 0.089997F;
        sTP_DEV.iXoff = 516L;
        sTP_DEV.iYoff = -22L;
    }
    else if(sTP_DEV.TP_Scan_Dir == L2R_U2D)
    {
        sTP_DEV.fXfac = 0.089697F;
        sTP_DEV.fYfac = 0.134792F;
        sTP_DEV.iXoff = -21L;
        sTP_DEV.iYoff = -39L;
    }
    else if(sTP_DEV.TP_Scan_Dir == R2L_D2U)
    {
        sTP_DEV.fXfac = 0.089915F;
        sTP_DEV.fYfac =  0.133178F;
        sTP_DEV.iXoff = -22L;
        sTP_DEV.iYoff = -38L;
    }
    else if(sTP_DEV.TP_Scan_Dir == U2D_R2L)
    {
        sTP_DEV.fXfac = -0.132906F;
        sTP_DEV.fYfac = 0.087964F;
        sTP_DEV.iXoff = 517L;
        sTP_DEV.iYoff = -20L;
    }

    gpio_pad_select_gpio(LCD_PIN_IRQ);
    gpio_set_direction(LCD_PIN_IRQ, GPIO_MODE_INPUT);
    gpio_set_intr_type(LCD_PIN_IRQ, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(LCD_PIN_IRQ, debounce_isr, (void*) NULL);
}

void touch_input_keyboard(void)
{
    const char *keyboard_str = "123456789<0>";
    if(sTP_DEV.chStatus & TP_PRESS_DOWN)
    {
        for(int j = 0; j < 4;j++)
        {
            for(int i = 0; i < 3;i++)
            {
                if(sTP_Draw.Xpoint >= QR_OFFSET +     i*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/3 &&
                   sTP_Draw.Xpoint <= QR_OFFSET + (i+1)*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/3 &&
                   sTP_Draw.Ypoint >= sLCD_DIS.LCD_Dis_Page/2 +     j*40 - 20 &&
                   sTP_Draw.Ypoint <= sLCD_DIS.LCD_Dis_Page/2 + (j+1)*40 - 20)
                {
                    if(*(keyboard_str + i + 3*j) != '<' && keyboard_pos < 15)
                    {
                        keyboard_input_str[keyboard_pos++] = *(keyboard_str + i + 3*j);
                    }
                    if(*(keyboard_str + i + 3*j) == '<' && keyboard_pos > 0)
                    {
                        screen_draw_rectangle(sLCD_DIS.LCD_Dis_Column/2 - Font24.Width*keyboard_pos/2,
                                              QR_OFFSET*3,
                                              sLCD_DIS.LCD_Dis_Column/2 + Font24.Width*keyboard_pos/2,
                                              QR_OFFSET*3 + Font24.Height,
                                              LCD_WHITE, DRAW_FULL, DOT_PIXEL_1X1);
                        keyboard_input_str[--keyboard_pos] = '\0';
                    }
                    screen_print_text(sLCD_DIS.LCD_Dis_Column/2 - Font24.Width*keyboard_pos/2,
                                      QR_OFFSET*3,
                                      keyboard_input_str,
                                      &Font24,
                                      LCD_WHITE,
                                      LCD_BLACK);
                }
            }
        }
    }
}

void IRAM_ATTR debounce_task(void *arg)
{
    uint8_t state = 0;
    while(1)
    {
        if(xQueueReceive(debounce_queue, &debounce_touch, 150/portTICK_PERIOD_MS))
        {
            state = 1;
        }
        else if(state == 1)
        {
            gpio_isr_handler_remove(LCD_PIN_IRQ);
            state = 0;
            touch_input();
            touch_input_keyboard();
            gpio_isr_handler_add(LCD_PIN_IRQ, debounce_isr, (void*) NULL);
        }
    }
}