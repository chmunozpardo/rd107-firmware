#include "handler_screen.h"
#include "Waveshare_ILI9486.h"
#include "qrcodegen.h"

static const char *TAG ="qr_handler";

extern LCD_DIS sLCD_DIS;

static char status[6] = "123456";

void GUI_Swop(POINT Point1, POINT Point2)
{
    POINT Temp;
    Temp = Point1;
    Point1 = Point2;
    Point2 = Temp;
}

void GUI_DrawPoint(POINT Xpoint, POINT Ypoint, COLOR Color,
                   DOT_PIXEL Dot_Pixel, DOT_STYLE DOT_STYLE)
{
    if (Xpoint > sLCD_DIS.LCD_Dis_Column || Ypoint > sLCD_DIS.LCD_Dis_Page) {
        ESP_LOGD(TAG, "GUI_DrawPoint Input exceeds the normal display range\r\n");
        return;
    }

    int16_t XDir_Num , YDir_Num;
    if (DOT_STYLE == DOT_STYLE_DFT)
    {
        for (XDir_Num = 0; XDir_Num < 2 * Dot_Pixel - 1; XDir_Num++)
        {
            for (YDir_Num = 0; YDir_Num < 2 * Dot_Pixel - 1; YDir_Num++) LCD_SetPointlColor(Xpoint + XDir_Num - Dot_Pixel + 1, Ypoint + YDir_Num - Dot_Pixel + 1, Color);
        }
    }
    else
    {
        for (XDir_Num = 0; XDir_Num <  Dot_Pixel; XDir_Num++)
        {
            for (YDir_Num = 0; YDir_Num <  Dot_Pixel; YDir_Num++) LCD_SetPointlColor(Xpoint + XDir_Num, Ypoint + YDir_Num, Color);
        }
    }
}

void GUI_DrawLine(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
                  COLOR Color, LINE_STYLE Line_Style, DOT_PIXEL Dot_Pixel)
{
    if (Xstart > sLCD_DIS.LCD_Dis_Column || Ystart > sLCD_DIS.LCD_Dis_Page ||
        Xend > sLCD_DIS.LCD_Dis_Column || Yend > sLCD_DIS.LCD_Dis_Page)
    {
        ESP_LOGD(TAG, "GUI_DrawLine Input exceeds the normal display range\r\n");
        return;
    }

    if (Xstart > Xend) GUI_Swop(Xstart, Xend);
    if (Ystart > Yend) GUI_Swop(Ystart, Yend);

    POINT Xpoint = Xstart;
    POINT Ypoint = Ystart;
    int dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
    int dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;
    int XAddway = Xstart < Xend ? 1 : -1;
    int YAddway = Ystart < Yend ? 1 : -1;
    int Esp = dx + dy;
    char Line_Style_Temp = 0;

    for (;;)
    {
        Line_Style_Temp++;
        if (Line_Style == LINE_DOTTED && Line_Style_Temp % 3 == 0)
        {
            ESP_LOGD(TAG, "LINE_DOTTED\r\n");
            GUI_DrawPoint(Xpoint, Ypoint, LCD_BACKGROUND, Dot_Pixel, DOT_STYLE_DFT);
            Line_Style_Temp = 0;
        }
        else
        {
            GUI_DrawPoint(Xpoint, Ypoint, Color, Dot_Pixel, DOT_STYLE_DFT);
        }
        if (2 * Esp >= dy)
        {
            if (Xpoint == Xend) break;
            Esp += dy;
            Xpoint += XAddway;
        }
        if (2 * Esp <= dx)
        {
            if (Ypoint == Yend) break;
            Esp += dx;
            Ypoint += YAddway;
        }
    }
}

void GUI_DrawRectangle(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
                       COLOR Color, DRAW_FILL Filled, DOT_PIXEL Dot_Pixel)
{
    if (Xstart > sLCD_DIS.LCD_Dis_Column || Ystart > sLCD_DIS.LCD_Dis_Page ||
      Xend > sLCD_DIS.LCD_Dis_Column || Yend > sLCD_DIS.LCD_Dis_Page)
    {
        ESP_LOGD(TAG, "Input exceeds the normal display range\r\n");
        return;
    }

    if (Xstart > Xend) GUI_Swop(Xstart, Xend);
    if (Ystart > Yend) GUI_Swop(Ystart, Yend);

    if (Filled)
    {
        LCD_SetArealColor(Xstart, Ystart, Xend, Yend, Color);
    }
    else
    {
        GUI_DrawLine(Xstart, Ystart, Xend, Ystart, Color , LINE_SOLID, Dot_Pixel);
        GUI_DrawLine(Xstart, Ystart, Xstart, Yend, Color , LINE_SOLID, Dot_Pixel);
        GUI_DrawLine(Xend, Yend, Xend, Ystart, Color , LINE_SOLID, Dot_Pixel);
        GUI_DrawLine(Xend, Yend, Xstart, Yend, Color , LINE_SOLID, Dot_Pixel);
    }
}

void GUI_DrawCircle(POINT X_Center, POINT Y_Center, LENGTH Radius,
                    COLOR Color, DRAW_FILL  Draw_Fill , DOT_PIXEL Dot_Pixel)
{
    if (X_Center > sLCD_DIS.LCD_Dis_Column || Y_Center >= sLCD_DIS.LCD_Dis_Page) {
        ESP_LOGD(TAG, "GUI_DrawCircle Input exceeds the normal display range\r\n");
        return;
    }

    int16_t XCurrent, YCurrent;
    XCurrent = 0;
    YCurrent = Radius;
    int16_t Esp = 3 - (Radius << 1 );
    int16_t sCountY;
    if (Draw_Fill == DRAW_FULL)
    {
        while (XCurrent <= YCurrent )
        {
            for (sCountY = XCurrent; sCountY <= YCurrent; sCountY ++ )
            {
                GUI_DrawPoint(X_Center + XCurrent, Y_Center + sCountY , Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                GUI_DrawPoint(X_Center - XCurrent, Y_Center + sCountY , Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                GUI_DrawPoint(X_Center - sCountY , Y_Center + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                GUI_DrawPoint(X_Center - sCountY , Y_Center - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                GUI_DrawPoint(X_Center - XCurrent, Y_Center - sCountY , Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                GUI_DrawPoint(X_Center + XCurrent, Y_Center - sCountY , Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                GUI_DrawPoint(X_Center + sCountY , Y_Center - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                GUI_DrawPoint(X_Center + sCountY , Y_Center + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
            }
            if (Esp < 0 ) Esp += 4 * XCurrent + 6;
            else
            {
                Esp += 10 + 4 * (XCurrent - YCurrent);
                YCurrent --;
            }
            XCurrent++;
        }
    }
    else
    {
        while(XCurrent <= YCurrent)
        {
            GUI_DrawPoint(X_Center + XCurrent, Y_Center + YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );
            GUI_DrawPoint(X_Center - XCurrent, Y_Center + YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );
            GUI_DrawPoint(X_Center - YCurrent, Y_Center + XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );
            GUI_DrawPoint(X_Center - YCurrent, Y_Center - XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );
            GUI_DrawPoint(X_Center - XCurrent, Y_Center - YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );
            GUI_DrawPoint(X_Center + XCurrent, Y_Center - YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );
            GUI_DrawPoint(X_Center + YCurrent, Y_Center - XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );
            GUI_DrawPoint(X_Center + YCurrent, Y_Center + XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT );
            if (Esp < 0 ) Esp += 4 * XCurrent + 6;
            else
            {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent--;
            }
            XCurrent ++;
        }
    }
}

void GUI_DisChar(POINT Xpoint, POINT Ypoint, const char Acsii_Char,
                 sFONT *Font, COLOR Color_Background, COLOR Color_Foreground)
{
    POINT Page, Column;

    if (Xpoint > sLCD_DIS.LCD_Dis_Column || Ypoint > sLCD_DIS.LCD_Dis_Page) {
        ESP_LOGD(TAG, "GUI_DisChar Input exceeds the normal display range\r\n");
        return;
    }

    uint32_t Char_Offset = (Acsii_Char - ' ') * Font->Height * (Font->Width / 8 + (Font->Width % 8 ? 1 : 0));
    const unsigned char *ptr = &Font->table[Char_Offset];

    for (Page = 0; Page < Font->Height; Page++)
    {
        for (Column = 0; Column < Font->Width; Column++)
        {
            if (FONT_BACKGROUND == Color_Background)
            {
                if (*ptr & (0x80 >> (Column % 8))) GUI_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Foreground, DOT_PIXEL_DFT, DOT_STYLE_DFT);
            }
            else
            {
                if (*ptr & (0x80 >> (Column % 8))) GUI_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Foreground, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                else GUI_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Background, DOT_PIXEL_DFT, DOT_STYLE_DFT);
            }
            if (Column % 8 == 7) ptr++;
        }
        if (Font->Width % 8 != 0) ptr++;
    }
}

void GUI_DisString_EN(POINT Xstart, POINT Ystart, const char * pString,
                      sFONT *Font, COLOR Color_Background, COLOR Color_Foreground)
{
    POINT Xpoint = Xstart;
    POINT Ypoint = Ystart;

    if (Xstart > sLCD_DIS.LCD_Dis_Column || Ystart > sLCD_DIS.LCD_Dis_Page) {
        ESP_LOGD(TAG, "GUI_DisString_EN Input exceeds the normal display range\r\n");
        return;
    }

    while (* pString != '\0')
    {
        if ((Xpoint + Font->Width ) > sLCD_DIS.LCD_Dis_Column || *pString == '\n')
        {
            Xpoint = Xstart;
            Ypoint += Font->Height;
        }
        if ((Ypoint  + Font->Height ) > sLCD_DIS.LCD_Dis_Page )
        {
            Xpoint = Xstart;
            Ypoint = Ystart;
        }
        if(*pString != '\n')
        {
            GUI_DisChar(Xpoint, Ypoint, * pString, Font, Color_Background, Color_Foreground);
            Xpoint += Font->Width;
        }
        pString ++;
    }
}

#define  ARRAY_LEN 255
void GUI_DisNum(POINT Xpoint, POINT Ypoint, int32_t Nummber,
                sFONT *Font, COLOR Color_Background, COLOR Color_Foreground )
{

    int16_t Num_Bit = 0, Str_Bit = 0;
    uint8_t Str_Array[ARRAY_LEN] = {0}, Num_Array[ARRAY_LEN] = {0};
    uint8_t *pStr = Str_Array;

    if (Xpoint > sLCD_DIS.LCD_Dis_Column || Ypoint > sLCD_DIS.LCD_Dis_Page)
    {
        ESP_LOGD(TAG, "GUI_DisNum Input exceeds the normal display range\r\n");
        return;
    }

    while (Nummber)
    {
        Num_Array[Num_Bit] = Nummber % 10 + '0';
        Num_Bit++;
        Nummber /= 10;
    }

    while (Num_Bit > 0)
    {
        Str_Array[Str_Bit] = Num_Array[Num_Bit - 1];
        Str_Bit ++;
        Num_Bit --;
    }

    GUI_DisString_EN(Xpoint, Ypoint, (const char*)pStr, Font, Color_Background, Color_Foreground );
}

sFONT *GUI_GetFontSize(POINT Dx, POINT Dy)
{
    sFONT *Font = NULL;
    if (Dx > Font24.Width && Dy > Font24.Height) Font = &Font24;
    else if ((Dx > Font20.Width && Dx < Font24.Width) && (Dy > Font20.Height && Dy < Font24.Height)) Font = &Font20;
    else if ((Dx > Font16.Width && Dx < Font20.Width) && (Dy > Font16.Height && Dy < Font20.Height)) Font = &Font16;
    else if ((Dx > Font12.Width && Dx < Font16.Width) && (Dy > Font12.Height && Dy < Font16.Height)) Font = &Font12;
    else if ((Dx > Font8.Width  && Dx < Font12.Width) && (Dy > Font8.Height  && Dy < Font12.Height)) Font = &Font8;
    else ESP_LOGD(TAG, "Please change the display area size, or add a larger font to modify\r\n");
    return Font;
}

void screen_logo(POINT Xpoint, POINT Ypoint, COLOR Color_Background, COLOR Color_Foreground)
{
    POINT Page, Column;

    if (Xpoint > sLCD_DIS.LCD_Dis_Column || Ypoint > sLCD_DIS.LCD_Dis_Page) {
        ESP_LOGD(TAG, "GUI_DisChar Input exceeds the normal display range\r\n");
        return;
    }

    const unsigned char *ptr = dreamit_LOGO_Top.table;

    for (Page = 0; Page < dreamit_LOGO_Top.Height; Page++)
    {
        for (Column = 0; Column < dreamit_LOGO_Top.Width; Column++)
        {
            if (FONT_BACKGROUND == Color_Background)
            {
                if (*ptr & (0x80 >> (Column % 8))) GUI_DrawPoint(Xpoint + Column, Ypoint + Page, LCD_LOGO_TOP, DOT_PIXEL_DFT, DOT_STYLE_DFT);
            }
            else
            {
                if (*ptr & (0x80 >> (Column % 8))) GUI_DrawPoint(Xpoint + Column, Ypoint + Page, LCD_LOGO_TOP, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                else GUI_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Background, DOT_PIXEL_DFT, DOT_STYLE_DFT);
            }
            if (Column % 8 == 7) ptr++;
        }
        if (dreamit_LOGO_Top.Width % 8 != 0) ptr++;
    }

    ptr = dreamit_LOGO_Bot.table;

    for (Page = 0; Page < dreamit_LOGO_Bot.Height; Page++)
    {
        for (Column = 0; Column < dreamit_LOGO_Bot.Width; Column++)
        {
            if (FONT_BACKGROUND == Color_Background)
            {
                if (*ptr & (0x80 >> (Column % 8))) GUI_DrawPoint(Xpoint + Column, Ypoint + Page, LCD_LOGO_BOT, DOT_PIXEL_DFT, DOT_STYLE_DFT);
            }
            else
            {
                if (*ptr & (0x80 >> (Column % 8))) GUI_DrawPoint(Xpoint + Column, Ypoint + Page, LCD_LOGO_BOT, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                else GUI_DrawPoint(Xpoint + Column, Ypoint + Page, Color_Background, DOT_PIXEL_DFT, DOT_STYLE_DFT);
            }
            if (Column % 8 == 7) ptr++;
        }
        if (dreamit_LOGO_Bot.Width % 8 != 0) ptr++;
    }
}

void screen_printf(char *text)
{
    screen_clear(LCD_BACKGROUND);
    GUI_DisString_EN(10, 80, text, &Font24, FONT_BACKGROUND, FONT_FOREGROUND);
    screen_logo(10, 10, FONT_BACKGROUND, FONT_FOREGROUND);
}

void qr_task(void *arg)
{
    screen_clear(LCD_BACKGROUND);
    enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;
    uint8_t qrcode[qrcodegen_BUFFER_LEN_FOR_VERSION(2)];
    uint8_t temp_buffer[qrcodegen_BUFFER_LEN_FOR_VERSION(2)];
    while(1)
    {
        if(xQueueReceive(qr_task_queue, &status, portMAX_DELAY))
        {
            qrcodegen_encodeText(status,
                                 temp_buffer,
                                 qrcode,
                                 errCorLvl,
                                 qrcodegen_VERSION_MIN,
                                 qrcodegen_VERSION_MAX,
                                 qrcodegen_Mask_AUTO,
                                 true);
            int size = qrcodegen_getSize(qrcode);

            for(int i = 0; i < size; i++)
            {
                for(int j = 0; j < size; j++)
                {
                    if(qrcodegen_getModule(qrcode, i, j))
                        GUI_DrawRectangle(   i*QR_SIZE + QR_OFFSET,
                                             j*QR_SIZE + QR_OFFSET,
                                         (i+1)*QR_SIZE + QR_OFFSET,
                                         (j+1)*QR_SIZE + QR_OFFSET,
                                         LCD_BLACK, DRAW_FULL, DOT_PIXEL_DFT);
                    else
                        GUI_DrawRectangle(   i*QR_SIZE + QR_OFFSET,
                                             j*QR_SIZE + QR_OFFSET,
                                         (i+1)*QR_SIZE + QR_OFFSET,
                                         (j+1)*QR_SIZE + QR_OFFSET,
                                         LCD_WHITE, DRAW_FULL, DOT_PIXEL_DFT);
                }
            }
        }
    }
}