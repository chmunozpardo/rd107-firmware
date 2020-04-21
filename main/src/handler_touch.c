#include "handler_spi.h"
#include "handler_touch.h"
#include "handler_screen.h"

#define READ_TIMES  4
#define LOST_NUM    0
#define ERR_RANGE   50

static TP_DEV sTP_DEV;
static TP_DRAW sTP_Draw;

static portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

static uint64_t prevT = 0;
static uint64_t currT = 0;

static IRAM_ATTR void touch_isr(void* arg)
{
    currT = esp_timer_get_time();
    portENTER_CRITICAL_ISR(&timerMux);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    if(!gpio_get_level(LCD_PIN_IRQ))
    {
        ets_printf("Diff = %llu\n", currT - prevT);
    }
    portEXIT_CRITICAL_ISR(&timerMux);
    prevT = currT;
}

static uint16_t TP_Read_ADC(unsigned char CMD)
{
    uint16_t Data = 0;
    Data = screen_read_byte(CMD);
    return Data;
}

static uint16_t TP_Read_ADC_Average(unsigned char Channel_Cmd)
{
    unsigned char i;
    uint16_t Read_Sum = 0, Read_Temp = 0;

    for (i = 0; i < READ_TIMES; i++) Read_Sum += TP_Read_ADC(Channel_Cmd);
    Read_Temp = Read_Sum / READ_TIMES;
    return Read_Temp;
}

static void TP_Read_ADC_XY(uint16_t *pXCh_Adc, uint16_t *pYCh_Adc)
{
    *pXCh_Adc = TP_Read_ADC_Average(0xDC);
    *pYCh_Adc = TP_Read_ADC_Average(0x9C);
}

static bool TP_Read_TwiceADC(uint16_t *pXCh_Adc, uint16_t *pYCh_Adc)
{
    uint16_t XCh_Adc1, YCh_Adc1, XCh_Adc2, YCh_Adc2;

    TP_Read_ADC_XY(&XCh_Adc1, &YCh_Adc1);
    TP_Read_ADC_XY(&XCh_Adc2, &YCh_Adc2);

    if( ((XCh_Adc2 <= XCh_Adc1 && XCh_Adc1 < XCh_Adc2 + ERR_RANGE) ||
        (XCh_Adc1 <= XCh_Adc2 && XCh_Adc2 < XCh_Adc1 + ERR_RANGE))
       && ((YCh_Adc2 <= YCh_Adc1 && YCh_Adc1 < YCh_Adc2 + ERR_RANGE) ||
           (YCh_Adc1 <= YCh_Adc2 && YCh_Adc2 < YCh_Adc1 + ERR_RANGE)))
    {
        *pXCh_Adc = (XCh_Adc1 + XCh_Adc2) / 2;
        *pYCh_Adc = (YCh_Adc1 + YCh_Adc2) / 2;
        return true;
    }
    return false;
}

static unsigned char TP_Scan(unsigned char chCoordType)
{
    if(!gpio_get_level(LCD_PIN_IRQ))
    {
        if(chCoordType)
        {
            TP_Read_TwiceADC(&sTP_DEV.Xpoint, &sTP_DEV.Ypoint);
        }
        else if(TP_Read_TwiceADC(&sTP_DEV.Xpoint, &sTP_DEV.Ypoint))
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

void touch_init(void)
{
    sTP_DEV.TP_Scan_Dir = SCAN_DIR_DFT;

    if( sTP_DEV.TP_Scan_Dir == D2U_L2R)
    {
        sTP_DEV.fXfac = -0.132443F;
        sTP_DEV.fYfac = 0.089997F;
        sTP_DEV.iXoff = 516L;
        sTP_DEV.iYoff = -22L;
    }
    else if( sTP_DEV.TP_Scan_Dir == L2R_U2D)
    {
        sTP_DEV.fXfac = 0.089697F;
        sTP_DEV.fYfac = 0.134792F;
        sTP_DEV.iXoff = -21L;
        sTP_DEV.iYoff = -39L;
    }
    else if( sTP_DEV.TP_Scan_Dir == R2L_D2U)
    {
        sTP_DEV.fXfac = 0.089915F;
        sTP_DEV.fYfac =  0.133178F;
        sTP_DEV.iXoff = -22L;
        sTP_DEV.iYoff = -38L;
    }
    else if( sTP_DEV.TP_Scan_Dir == U2D_R2L)
    {
        sTP_DEV.fXfac = -0.132906F;
        sTP_DEV.fYfac = 0.087964F;
        sTP_DEV.iXoff = 517L;
        sTP_DEV.iYoff = -20L;
    }

    TP_Read_ADC_XY(&sTP_DEV.Xpoint, &sTP_DEV.Ypoint);

    gpio_pad_select_gpio(LCD_PIN_IRQ);
    gpio_set_direction(LCD_PIN_IRQ, GPIO_MODE_INPUT);
    gpio_set_intr_type(LCD_PIN_IRQ, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(LCD_PIN_IRQ, touch_isr, (void*) NULL);
}

void TP_Dialog(void)
{
    screen_clear(LCD_BACKGROUND);
    if(sLCD_DIS.LCD_Dis_Column > sLCD_DIS.LCD_Dis_Page)
    {
        screen_print_text(sLCD_DIS.LCD_Dis_Column - 60, 0,
                         "CLEAR", &Font16, LCD_RED, LCD_BLUE);
        screen_print_text(sLCD_DIS.LCD_Dis_Column - 120, 0,
                         "AD", &Font24, LCD_RED, LCD_BLUE);
        screen_draw_rectangle(sLCD_DIS.LCD_Dis_Column - 50, 20,
                              sLCD_DIS.LCD_Dis_Column, 70,
                              LCD_BLUE, DRAW_FULL, DOT_PIXEL_1X1);
        screen_draw_rectangle(sLCD_DIS.LCD_Dis_Column - 50, 80,
                              sLCD_DIS.LCD_Dis_Column, 130,
                              LCD_GREEN, DRAW_FULL, DOT_PIXEL_1X1);
        screen_draw_rectangle(sLCD_DIS.LCD_Dis_Column - 50, 140,
                              sLCD_DIS.LCD_Dis_Column, 190,
                              LCD_RED, DRAW_FULL, DOT_PIXEL_1X1);
        screen_draw_rectangle(sLCD_DIS.LCD_Dis_Column - 50, 200,
                              sLCD_DIS.LCD_Dis_Column, 250,
                              LCD_YELLOW, DRAW_FULL, DOT_PIXEL_1X1);
        screen_draw_rectangle(sLCD_DIS.LCD_Dis_Column - 50, 260,
                              sLCD_DIS.LCD_Dis_Column, 310,
                              LCD_BLACK, DRAW_FULL, DOT_PIXEL_1X1);
    }
    else
    {
        screen_print_text(sLCD_DIS.LCD_Dis_Column - 60, 0,
                         "CLEAR", &Font16, LCD_RED, LCD_BLUE);
        screen_print_text(sLCD_DIS.LCD_Dis_Column - 120, 0,
                         "AD", &Font24, LCD_RED, LCD_BLUE);
        screen_draw_rectangle(20, 20, 70, 70, LCD_BLUE, DRAW_FULL, DOT_PIXEL_1X1);
        screen_draw_rectangle(80, 20, 130, 70, LCD_GREEN, DRAW_FULL, DOT_PIXEL_1X1);
        screen_draw_rectangle(140, 20, 190, 70, LCD_RED, DRAW_FULL, DOT_PIXEL_1X1);
        screen_draw_rectangle(200, 20, 250, 70, LCD_YELLOW, DRAW_FULL, DOT_PIXEL_1X1);
        screen_draw_rectangle(260, 20, 310, 70, LCD_BLACK, DRAW_FULL, DOT_PIXEL_1X1);
    }
}

void TP_DrawBoard(void)
{
    TP_Scan(0);
    if(sTP_DEV.chStatus & TP_PRESS_DOWN)
    {
        if(sTP_Draw.Xpoint < sLCD_DIS.LCD_Dis_Column &&
           sTP_Draw.Ypoint < sLCD_DIS.LCD_Dis_Page)
        {
            if(sLCD_DIS.LCD_Dis_Column > sLCD_DIS.LCD_Dis_Page)
            {
                if(sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 60) &&
                   sTP_Draw.Ypoint < 16)
                {
                    TP_Dialog();
                }
                else if(sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 120) &&
                        sTP_Draw.Xpoint < (sLCD_DIS.LCD_Dis_Column -  80) &&
                        sTP_Draw.Ypoint < 24)
                {
                    TP_Dialog();
                } 
                else if(sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 50) &&
                        sTP_Draw.Xpoint < sLCD_DIS.LCD_Dis_Column &&
                        sTP_Draw.Ypoint > 20 &&
                        sTP_Draw.Ypoint < 70)
                {
                    sTP_Draw.Color = LCD_BLUE;
                }
                else if(sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 50) &&
                        sTP_Draw.Xpoint < sLCD_DIS.LCD_Dis_Column &&
                        sTP_Draw.Ypoint > 80 &&
                        sTP_Draw.Ypoint < 130)
                {
                    sTP_Draw.Color = LCD_GREEN;
                }
                else if(sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 50) &&
                        sTP_Draw.Xpoint < sLCD_DIS.LCD_Dis_Column &&
                        sTP_Draw.Ypoint > 140 &&
                        sTP_Draw.Ypoint < 190)
                {
                    sTP_Draw.Color = LCD_RED;
                }
                else if(sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 50) &&
                        sTP_Draw.Xpoint < sLCD_DIS.LCD_Dis_Column &&
                        sTP_Draw.Ypoint > 200 && sTP_Draw.Ypoint < 250)
                {
                    sTP_Draw.Color = LCD_YELLOW;
                }
                else if(sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 50) &&
                        sTP_Draw.Xpoint < sLCD_DIS.LCD_Dis_Column &&
                        sTP_Draw.Ypoint > 260 &&
                        sTP_Draw.Ypoint < 310)
                {
                    sTP_Draw.Color = LCD_BLACK;
                }
                else
                {
                    screen_draw_point(sTP_Draw.Xpoint, sTP_Draw.Ypoint,
                                      sTP_Draw.Color , DOT_PIXEL_1X1, DOT_FILL_RIGHTUP);
                    screen_draw_point(sTP_Draw.Xpoint + 1, sTP_Draw.Ypoint,
                                      sTP_Draw.Color , DOT_PIXEL_1X1, DOT_FILL_RIGHTUP);
                    screen_draw_point(sTP_Draw.Xpoint, sTP_Draw.Ypoint + 1,
                                      sTP_Draw.Color , DOT_PIXEL_1X1, DOT_FILL_RIGHTUP);
                    screen_draw_point(sTP_Draw.Xpoint + 1, sTP_Draw.Ypoint + 1,
                                      sTP_Draw.Color , DOT_PIXEL_1X1, DOT_FILL_RIGHTUP);
                    screen_draw_point(sTP_Draw.Xpoint, sTP_Draw.Ypoint,
                                      sTP_Draw.Color , DOT_PIXEL_2X2, DOT_FILL_RIGHTUP);
                }
            }
            else
            {
                if(sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 60) &&
                   sTP_Draw.Ypoint < 16)
                {
                    TP_Dialog();
                }
                else if(sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 120) &&
                        sTP_Draw.Xpoint < (sLCD_DIS.LCD_Dis_Column -  80) &&
                        sTP_Draw.Ypoint < 24) {
                    TP_Dialog();
                }
                else if(sTP_Draw.Xpoint > 20 && sTP_Draw.Xpoint < 70 &&
                        sTP_Draw.Ypoint > 20 && sTP_Draw.Ypoint < 70)
                {
                    sTP_Draw.Color = LCD_BLUE;
                }
                else if(sTP_Draw.Xpoint > 80 && sTP_Draw.Xpoint < 130 &&
                        sTP_Draw.Ypoint > 20 && sTP_Draw.Ypoint <  70)
                {
                    sTP_Draw.Color = LCD_GREEN;
                }
                else if(sTP_Draw.Xpoint > 140 && sTP_Draw.Xpoint < 190 &&
                        sTP_Draw.Ypoint >  20 && sTP_Draw.Ypoint <  70)
                {
                    sTP_Draw.Color = LCD_RED;
                }
                else if(sTP_Draw.Xpoint > 200 && sTP_Draw.Xpoint < 250 &&
                        sTP_Draw.Ypoint >  20 && sTP_Draw.Ypoint <  70)
                {
                    sTP_Draw.Color = LCD_YELLOW;
                }
                else if(sTP_Draw.Xpoint > 260 && sTP_Draw.Xpoint < 310 &&
                        sTP_Draw.Ypoint >  20 && sTP_Draw.Ypoint <  70)
                {
                    sTP_Draw.Color = LCD_BLACK;
                }
                else
                {
                    screen_draw_point(sTP_Draw.Xpoint, sTP_Draw.Ypoint,
                                      sTP_Draw.Color , DOT_PIXEL_2X2,
                                      DOT_FILL_RIGHTUP);
                }
            }
        }
    }
}