#include "handler_spi.h"
#include "handler_touch.h"
#include "handler_search.h"
#include "handler_screen.h"

static DRAM_ATTR xQueueHandle debounce_queue = NULL;

static TP_DEV sTP_DEV;
static TP_DRAW sTP_Draw;
static void *touch_context = NULL;

static DRAM_ATTR uint8_t debounce_touch = 0;

static char keyboard_input_str[32] = "";

static uint8_t wifi_list    = 0;
static uint8_t keyboard_pos = 0;
static uint8_t keyboard_min = 0;

static char digito_verificador_rut(char *rut_str)
{
   uint32_t sum = 0, factor = 2;
   uint32_t rut = atoi(rut_str);
   while(rut)
   {
       sum   += (rut%10)*factor;
       rut   /= 10;
       factor = factor==7 ? 2 : factor+1;
   }
   const uint32_t res = 11 - sum%11;
   return res == 11? '0' : res == 10? 'k' : res+'0';
}

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
    if(sTP_DEV.chStatus & TP_PRESS_DOWN)
    {
        if(touch_context_status == TOUCH_NONE)
        {
            return;
        }
        else if(touch_context_status == TOUCH_SET_WIFI)
        {
            if(sTP_Draw.Xpoint >= sLCD_DIS.LCD_Dis_Column - button_Sign.Width - 10 &&
               sTP_Draw.Xpoint <= sLCD_DIS.LCD_Dis_Column -                 0 - 10 &&
               sTP_Draw.Ypoint >= 210 + 15 + 10 +   button_Sign.Height/2 + Font16.Height/2 &&
               sTP_Draw.Ypoint <= 210 + 15 + 10 + 3*button_Sign.Height/2 + Font16.Height/2)
            {
                *(((wifi_context_t*)touch_context)->opt)      = 0;
                *(((wifi_context_t*)touch_context)->http_ind) = 1;
                touch_context_status = TOUCH_NONE;
            }
            else if(sTP_Draw.Xpoint >= sLCD_DIS.LCD_Dis_Column - button_Sign.Width - 10 &&
                    sTP_Draw.Xpoint <= sLCD_DIS.LCD_Dis_Column -                 0 - 10 &&
                    sTP_Draw.Ypoint >= 210 + 15 - button_Sign.Height/2 + Font16.Height/2 &&
                    sTP_Draw.Ypoint <= 210 + 15 + button_Sign.Height/2 + Font16.Height/2)
            {
                touch_context_status = TOUCH_SET_WIFI_LIST;
                uint8_t tmp_list     = *(((wifi_context_t*)touch_context)->ap_count);
                wifi_list            = tmp_list <= 10 ? tmp_list : 10;
                screen_clear(LCD_WHITE);
                for(int i = 0; i < wifi_list; i++)
                    screen_print_text(10, 10 + i*(Font20.Height + 4),
                          (char*)((wifi_context_t*)touch_context)->ap_info[i].ssid, &Font20, LCD_WHITE, LCD_BLACK);

                screen_draw_from_rom(sLCD_DIS.LCD_Dis_Column - button_Sign.Width - 10,
                                     sLCD_DIS.LCD_Dis_Page - 10 - button_Sign.Height,
                                     &button_Sign, LCD_WHITE);
                screen_print_transp_text(sLCD_DIS.LCD_Dis_Column - button_Sign.Width/2 - 10 - Font24.Width*4/2,
                                         sLCD_DIS.LCD_Dis_Page - 10 - button_Sign.Height/2 - Font24.Height/2,
                                         "Back", &Font24, LCD_WHITE);
            }
        }
        else if(touch_context_status == TOUCH_SET_WIFI_LIST)
        {
            for(int i = 0; i < wifi_list; i++)
            {
                if(sTP_Draw.Xpoint >=                           10 &&
                   sTP_Draw.Xpoint <= sLCD_DIS.LCD_Dis_Column - 10 &&
                   sTP_Draw.Ypoint >= 10 +     i*(Font20.Height + 4) &&
                   sTP_Draw.Ypoint <= 10 + (i+1)*(Font20.Height + 4))
                {
                    wifi_list = i;
                    screen_clear(LCD_WHITE);
                    screen_print_text(10, 10,
                                      (char*)((wifi_context_t*)touch_context)->ap_info[i].ssid,
                                      &Font20,
                                      LCD_WHITE,
                                      LCD_BLACK);
                    screen_draw_keyboard(LCD_KEYBOARD_MAY);
                    touch_context_status = TOUCH_SET_WIFI_PASSWORD;
                }
            }
            if(sTP_Draw.Xpoint >= sLCD_DIS.LCD_Dis_Column - button_Sign.Width - 10 &&
               sTP_Draw.Xpoint <= sLCD_DIS.LCD_Dis_Column -                 0 - 10 &&
               sTP_Draw.Ypoint >= sLCD_DIS.LCD_Dis_Page - 10 - button_Sign.Height &&
               sTP_Draw.Ypoint <= sLCD_DIS.LCD_Dis_Page - 10 -                  0)
            {
                screen_print_conf("Connect to\nrd107ap2020\nand browse to\n192.168.4.1/config\nto configure Wi-Fi", true);
                touch_context_status = TOUCH_SET_WIFI;
            }
        }
        else if(touch_context_status == TOUCH_SET_WIFI_PASSWORD)
        {
            char *keyboard_str = (char*) malloc(41);
            if(keyboard_min == 0) strncpy(keyboard_str, LCD_KEYBOARD_MAY, 41);
            else strncpy(keyboard_str, LCD_KEYBOARD_MIN, 41);
            for(int j = 0; j < 4;j++)
            {
                for(int i = 0; i < 10;i++)
                {
                    if(sTP_Draw.Xpoint >= QR_OFFSET +     i*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/(10) &&
                       sTP_Draw.Xpoint <= QR_OFFSET + (i+1)*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/(10) &&
                       sTP_Draw.Ypoint >= sLCD_DIS.LCD_Dis_Page/2 +     j*40 - 20 &&
                       sTP_Draw.Ypoint <= sLCD_DIS.LCD_Dis_Page/2 + (j+1)*40 - 20)
                    {
                        if(*(keyboard_str + i + 10*j) == '_')
                        {
                            keyboard_min = 1;
                            screen_draw_keyboard(LCD_KEYBOARD_MIN);
                        }
                        if(*(keyboard_str + i + 10*j) == '^')
                        {
                            keyboard_min = 0;
                            screen_draw_keyboard(LCD_KEYBOARD_MAY);
                        }
                        if(*(keyboard_str + i + 10*j) == '>')
                        {
                            strcpy(((wifi_context_t*)touch_context)->wifi_password, keyboard_input_str);
                            *(((wifi_context_t*)touch_context)->opt)      = wifi_list + 1;
                            *(((wifi_context_t*)touch_context)->http_ind) = 1;
                            memset(keyboard_input_str, 0, 32);
                            keyboard_pos = 0;
                            touch_context_status = TOUCH_NONE;
                        }
                        if(*(keyboard_str + i + 10*j) == '<' &&
                           keyboard_pos > 0)
                        {
                            screen_draw_rectangle(sLCD_DIS.LCD_Dis_Column/2 - Font24.Width*keyboard_pos/2,
                                                  QR_OFFSET*3,
                                                  sLCD_DIS.LCD_Dis_Column/2 + Font24.Width*keyboard_pos/2,
                                                  QR_OFFSET*3 + Font24.Height,
                                                  LCD_WHITE, DRAW_FULL, DOT_PIXEL_1X1);
                            keyboard_input_str[--keyboard_pos] = '\0';
                            screen_print_text(sLCD_DIS.LCD_Dis_Column/2 - Font24.Width*keyboard_pos/2,
                                              QR_OFFSET*3,
                                              keyboard_input_str,
                                              &Font24,
                                              LCD_WHITE,
                                              LCD_BLACK);
                        }
                        if(*(keyboard_str + i + 10*j) != '<' &&
                           *(keyboard_str + i + 10*j) != '>' &&
                           *(keyboard_str + i + 10*j) != ' ' &&
                           *(keyboard_str + i + 10*j) != '_' &&
                           *(keyboard_str + i + 10*j) != '^' &&
                           keyboard_pos < 32)
                        {
                            keyboard_input_str[keyboard_pos++] = *(keyboard_str + i + 10*j);
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
        else if(touch_context_status == TOUCH_SET_DEVICE)
        {
            if(sTP_Draw.Xpoint >= sLCD_DIS.LCD_Dis_Column - button_Sign.Width - 10 &&
               sTP_Draw.Xpoint <= sLCD_DIS.LCD_Dis_Column -                 0 - 10 &&
               sTP_Draw.Ypoint >= 210 + 15 + 10 +   button_Sign.Height/2 + Font16.Height/2 &&
               sTP_Draw.Ypoint <= 210 + 15 + 10 + 3*button_Sign.Height/2 + Font16.Height/2)
            {
                *(((reg_context_t*)touch_context)->opt_web)  = 'y';
                *(((reg_context_t*)touch_context)->http_ind) =  1 ;
                touch_context_status = TOUCH_NONE;
            }
            else if(sTP_Draw.Xpoint >= sLCD_DIS.LCD_Dis_Column - button_Sign.Width - 10 &&
                    sTP_Draw.Xpoint <= sLCD_DIS.LCD_Dis_Column -                 0 - 10 &&
                    sTP_Draw.Ypoint >= 210 + 15 - button_Sign.Height/2 + Font16.Height/2 &&
                    sTP_Draw.Ypoint <= 210 + 15 + button_Sign.Height/2 + Font16.Height/2)
            {
                touch_context_status = TOUCH_SET_DEVICE_CODE;
                screen_draw_input_interface();
            }
        }
        else if(touch_context_status == TOUCH_SET_DEVICE_CODE)
        {
            const char *keyboard_str = "123456789<0>";
            for(int j = 0; j < 4;j++)
            {
                for(int i = 0; i < 3;i++)
                {
                    if(sTP_Draw.Xpoint >= QR_OFFSET +     i*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/(3) &&
                       sTP_Draw.Xpoint <= QR_OFFSET + (i+1)*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/(3) &&
                       sTP_Draw.Ypoint >= sLCD_DIS.LCD_Dis_Page/2 +     j*40 - 20 &&
                       sTP_Draw.Ypoint <= sLCD_DIS.LCD_Dis_Page/2 + (j+1)*40 - 20)
                    {
                        if(*(keyboard_str + i + 3*j) == '>')
                        {
                            strcpy(((reg_context_t*)touch_context)->registration_code, keyboard_input_str);
                            *(((reg_context_t*)touch_context)->opt_web)  = 'n';
                            *(((reg_context_t*)touch_context)->http_ind) =  1 ;
                            memset(keyboard_input_str, 0, 32);
                            keyboard_pos = 0;
                            touch_context_status = TOUCH_NONE;
                        }
                        if(*(keyboard_str + i + 3*j) == '<' &&
                           keyboard_pos > 0)
                        {
                            screen_draw_rectangle(sLCD_DIS.LCD_Dis_Column/2 - Font24.Width*(keyboard_pos)/2,
                                                  QR_OFFSET*3,
                                                  sLCD_DIS.LCD_Dis_Column/2 + Font24.Width*(keyboard_pos)/2,
                                                  QR_OFFSET*3 + Font24.Height,
                                                  LCD_WHITE, DRAW_FULL, DOT_PIXEL_1X1);
                            keyboard_input_str[--keyboard_pos] = '\0';
                            if(keyboard_pos > 0)
                            {
                                screen_print_text(sLCD_DIS.LCD_Dis_Column/2 - Font24.Width*(keyboard_pos)/2,
                                                  QR_OFFSET*3,
                                                  keyboard_input_str,
                                                  &Font24,
                                                  LCD_WHITE,
                                                  LCD_BLACK);
                            }
                        }
                        if(*(keyboard_str + i + 3*j) != '<' &&
                           *(keyboard_str + i + 3*j) != '>' &&
                           keyboard_pos < 32)
                        {
                            keyboard_input_str[keyboard_pos++] = *(keyboard_str + i + 3*j);
                            screen_print_text(sLCD_DIS.LCD_Dis_Column/2 - Font24.Width*(keyboard_pos)/2,
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
        else if(touch_context_status == TOUCH_QR_CODE)
        {
            if(sTP_Draw.Xpoint >= (sLCD_DIS.LCD_Dis_Column + QR_SIZE*21 + 2*QR_OFFSET)/2 - button_Sign.Width/2 &&
               sTP_Draw.Xpoint <= (sLCD_DIS.LCD_Dis_Column + QR_SIZE*21 + 2*QR_OFFSET)/2 + button_Sign.Width/2 &&
               sTP_Draw.Ypoint >= 2*QR_OFFSET + dreamit_LOGO_Big_Top.Height + 20 &&
               sTP_Draw.Ypoint <= 2*QR_OFFSET + dreamit_LOGO_Big_Top.Height + 20 + button_Sign.Height)
            {
                touch_context_status = TOUCH_INPUT_RESERVATION;
                SCREEN_SIGNAL("", 2, 0);
            }
            else if(sTP_Draw.Xpoint >= (sLCD_DIS.LCD_Dis_Column + QR_SIZE*21 + 2*QR_OFFSET)/2 - button_Sign.Width/2 &&
                    sTP_Draw.Xpoint <= (sLCD_DIS.LCD_Dis_Column + QR_SIZE*21 + 2*QR_OFFSET)/2 + button_Sign.Width/2 &&
                    sTP_Draw.Ypoint >= 2*QR_OFFSET + dreamit_LOGO_Big_Top.Height + 80 &&
                    sTP_Draw.Ypoint <= 2*QR_OFFSET + dreamit_LOGO_Big_Top.Height + 80 + button_Sign.Height)
            {
                touch_context_status = TOUCH_INPUT_RUT;
                SCREEN_SIGNAL("", 2, 0);
            }
        }
        else if(touch_context_status == TOUCH_INPUT_RESERVATION)
        {
            const char *keyboard_str = "1234567890QWERTYUIOPASDFGHJKL< ZXCVBNM >";
            for(int j = 0; j < 4;j++)
            {
                for(int i = 0; i < 10;i++)
                {
                    if(sTP_Draw.Xpoint >= QR_OFFSET +     i*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/(10) &&
                       sTP_Draw.Xpoint <= QR_OFFSET + (i+1)*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/(10) &&
                       sTP_Draw.Ypoint >= sLCD_DIS.LCD_Dis_Page/2 +     j*40 - 20 &&
                       sTP_Draw.Ypoint <= sLCD_DIS.LCD_Dis_Page/2 + (j+1)*40 - 20)
                    {
                        if(*(keyboard_str + i + 10*j) == '>')
                        {
                            touch_context_status = TOUCH_QR_CODE;
                            search_reservation_code(keyboard_input_str);
                            memset(keyboard_input_str, 0, 32);
                            keyboard_pos = 0;
                        }
                        if(*(keyboard_str + i + 10*j) == '<' &&
                           keyboard_pos > 0)
                        {
                            screen_draw_rectangle(sLCD_DIS.LCD_Dis_Column/2 - Font24.Width*keyboard_pos/2,
                                                  QR_OFFSET*3,
                                                  sLCD_DIS.LCD_Dis_Column/2 + Font24.Width*keyboard_pos/2,
                                                  QR_OFFSET*3 + Font24.Height,
                                                  LCD_WHITE, DRAW_FULL, DOT_PIXEL_1X1);
                            keyboard_input_str[--keyboard_pos] = '\0';
                            screen_print_text(sLCD_DIS.LCD_Dis_Column/2 - Font24.Width*keyboard_pos/2,
                                              QR_OFFSET*3,
                                              keyboard_input_str,
                                              &Font24,
                                              LCD_WHITE,
                                              LCD_BLACK);
                        }
                        if(*(keyboard_str + i + 10*j) != '<' &&
                           *(keyboard_str + i + 10*j) != '>' &&
                           *(keyboard_str + i + 10*j) != ' ' &&
                           keyboard_pos < 32)
                        {
                            keyboard_input_str[keyboard_pos++] = *(keyboard_str + i + 10*j);
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
        else if(touch_context_status == TOUCH_INPUT_RUT)
        {
            const char *keyboard_str = "123456789<0>";
            for(int j = 0; j < 4;j++)
            {
                for(int i = 0; i < 3;i++)
                {
                    if(sTP_Draw.Xpoint >= QR_OFFSET +     i*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/(3) &&
                       sTP_Draw.Xpoint <= QR_OFFSET + (i+1)*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/(3) &&
                       sTP_Draw.Ypoint >= sLCD_DIS.LCD_Dis_Page/2 +     j*40 - 20 &&
                       sTP_Draw.Ypoint <= sLCD_DIS.LCD_Dis_Page/2 + (j+1)*40 - 20)
                    {
                        if(*(keyboard_str + i + 3*j) == '>')
                        {
                            touch_context_status = TOUCH_QR_CODE;
                            search_rut(keyboard_input_str);
                            memset(keyboard_input_str, 0, 32);
                            keyboard_pos = 0;
                        }
                        if(*(keyboard_str + i + 3*j) == '<' &&
                           keyboard_pos > 0)
                        {
                            screen_draw_rectangle(sLCD_DIS.LCD_Dis_Column/2 - Font24.Width*(keyboard_pos+2)/2,
                                                  QR_OFFSET*3,
                                                  sLCD_DIS.LCD_Dis_Column/2 + Font24.Width*(keyboard_pos+2)/2,
                                                  QR_OFFSET*3 + Font24.Height,
                                                  LCD_WHITE, DRAW_FULL, DOT_PIXEL_1X1);
                            keyboard_input_str[--keyboard_pos] = '\0';
                            if(keyboard_pos > 0)
                            {
                                char digito          = digito_verificador_rut(keyboard_input_str);
                                char display_rut[18] = "";
                                strcpy(display_rut, keyboard_input_str);
                                strcat(display_rut, "-");
                                strncat(display_rut, &digito,1);
                                screen_print_text(sLCD_DIS.LCD_Dis_Column/2 - Font24.Width*(keyboard_pos+2)/2,
                                                  QR_OFFSET*3,
                                                  display_rut,
                                                  &Font24,
                                                  LCD_WHITE,
                                                  LCD_BLACK);
                            }
                        }
                        if(*(keyboard_str + i + 3*j) != '<' &&
                           *(keyboard_str + i + 3*j) != '>' &&
                           keyboard_pos < 32)
                        {
                            keyboard_input_str[keyboard_pos++] = *(keyboard_str + i + 3*j);
                            char digito          = digito_verificador_rut(keyboard_input_str);
                            char display_rut[18] = "";
                            strcpy(display_rut, keyboard_input_str);
                            strcat(display_rut, "-");
                            strncat(display_rut, &digito, 1);
                            screen_print_text(sLCD_DIS.LCD_Dis_Column/2 - Font24.Width*(keyboard_pos+2)/2,
                                              QR_OFFSET*3,
                                              display_rut,
                                              &Font24,
                                              LCD_WHITE,
                                              LCD_BLACK);
                        }
                    }
                }
            }
        }
    }
}

void IRAM_ATTR touch_set_context(void *context, uint8_t context_status)
{
    touch_context        = context;
    touch_context_status = context_status;
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