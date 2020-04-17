/*****************************************************************************
  | File          :   LCD_Touch.c
  | Author      :   Waveshare team
  | Function    :   LCD Touch Pad Driver and Draw
  | Info        :
    Image scanning
       Please use progressive scanning to generate images or fonts
  ----------------
  | This version:   V1.0
  | Date        :   2017-08-16
  | Info        :   Basic version

******************************************************************************/
#include "handler_spi.h"
#include "handler_touch.h"
#include "handler_screen.h"

extern LCD_DIS sLCD_DIS;
static TP_DEV sTP_DEV;
static TP_DRAW sTP_Draw;

/*******************************************************************************
  function:
        Read the ADC of the channel
  parameter:
    Channel_Cmd :   0x90: Read channel Y +, select the ADC resolution is 12 bits, set to differential mode
                    0xd0: Read channel x +, select the ADC resolution is 12 bits, set to differential mode
*******************************************************************************/
static uint16_t TP_Read_ADC(unsigned char CMD)
{
    uint16_t Data = 0;

    Data = screen_read_byte(CMD);

    return Data;
}

/*******************************************************************************
  function:
        Read the 5th channel value and exclude the maximum and minimum returns the average
  parameter:
    Channel_Cmd :   0x90 :Read channel Y +
                    0xd0 :Read channel x +
*******************************************************************************/
#define READ_TIMES  5   //Number of readings
#define LOST_NUM    1   //Discard value
static uint16_t TP_Read_ADC_Average(unsigned char Channel_Cmd)
{
  unsigned char i, j;
  uint16_t Read_Buff[READ_TIMES];
  uint16_t Read_Sum = 0, Read_Temp = 0;

  //Read and save multiple samples
  for (i = 0; i < READ_TIMES; i++) {
    Read_Buff[i] = TP_Read_ADC(Channel_Cmd);
    vTaskDelay(1/portTICK_PERIOD_MS);
  }

  //Sort from small to large
  for (i = 0; i < READ_TIMES  -  1; i ++) {
    for (j = i + 1; j < READ_TIMES; j ++) {
      if (Read_Buff[i] > Read_Buff[j]) {
        Read_Temp = Read_Buff[i];
        Read_Buff[i] = Read_Buff[j];
        Read_Buff[j] = Read_Temp;
      }
    }
  }

  //Exclude the largest and the smallest
  for (i = LOST_NUM; i < READ_TIMES - LOST_NUM; i ++)
    Read_Sum += Read_Buff[i];

  //Averaging
  Read_Temp = Read_Sum / (READ_TIMES - 2 * LOST_NUM);

  return Read_Temp;
}

/*******************************************************************************
  function:
        Read X channel and Y channel AD value
  parameter:
    Channel_Cmd :   0x90 :Read channel Y +
                    0xd0 :Read channel x +
*******************************************************************************/
static void TP_Read_ADC_XY(uint16_t *pXCh_Adc, uint16_t *pYCh_Adc )
{
    *pXCh_Adc = TP_Read_ADC_Average(0xDC);
    ESP_LOGI("TOUCH 0","%X", *pXCh_Adc);
    *pYCh_Adc = TP_Read_ADC_Average(0x9C);
    ESP_LOGI("TOUCH 1","%X", *pYCh_Adc);
    uint16_t asdf = TP_Read_ADC_Average(0x8C);
    ESP_LOGI("TOUCH 2","%X", asdf);
}

/*******************************************************************************
  function:
        2 times to read the touch screen IC, and the two can not exceed the deviation,
        ERR_RANGE, meet the conditions, then that the correct reading, otherwise the reading error.
  parameter:
    Channel_Cmd :   pYCh_Adc = 0x90 :Read channel Y +
                    pXCh_Adc = 0xd0 :Read channel x +
*******************************************************************************/
#define ERR_RANGE 50    //tolerance scope
static bool TP_Read_TwiceADC(uint16_t *pXCh_Adc, uint16_t *pYCh_Adc )
{
  uint16_t XCh_Adc1, YCh_Adc1, XCh_Adc2, YCh_Adc2;

  //Read the ADC values Read the ADC values twice
  TP_Read_ADC_XY(&XCh_Adc1, &YCh_Adc1);
  //  Driver_Delay_us(10);
  TP_Read_ADC_XY(&XCh_Adc2, &YCh_Adc2);
  //  Driver_Delay_us(10);

  //The ADC error used twice is greater than ERR_RANGE to take the average
  if ( ((XCh_Adc2 <= XCh_Adc1 && XCh_Adc1 < XCh_Adc2 + ERR_RANGE) ||
        (XCh_Adc1 <= XCh_Adc2 && XCh_Adc2 < XCh_Adc1 + ERR_RANGE))
       && ((YCh_Adc2 <= YCh_Adc1 && YCh_Adc1 < YCh_Adc2 + ERR_RANGE) ||
           (YCh_Adc1 <= YCh_Adc2 && YCh_Adc2 < YCh_Adc1 + ERR_RANGE))) {
    *pXCh_Adc = (XCh_Adc1 + XCh_Adc2) / 2;
    *pYCh_Adc = (YCh_Adc1 + YCh_Adc2) / 2;
    return true;
  }

  //The ADC error used twice is less than ERR_RANGE returns failed
  return false;
}

/*******************************************************************************
    function:
        Touch pad initialization
*******************************************************************************/
void TP_Init(LCD_SCAN_DIR Lcd_ScanDir)
{
    sTP_DEV.TP_Scan_Dir = Lcd_ScanDir;

    for(;;)
    {
        TP_Read_ADC_XY(&sTP_DEV.Xpoint, &sTP_DEV.Ypoint);
    }
}
