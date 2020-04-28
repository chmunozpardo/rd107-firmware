#include "handler_spi.h"
#include "handler_touch.h"
#include "handler_screen.h"
#include "qrcodegen.h"

static const char *TAG = "screen_handler";

screen_queue_t screen_task_data = {"123456\0", 0, 0};

LCD_DIS sLCD_DIS;

extern uint16_t tx_data_16b[SCREEN_BUFFER];

static void screen_swap(POINT Point1, POINT Point2)
{
    POINT Temp;
    Temp = Point1;
    Point1 = Point2;
    Point2 = Temp;
}

static void screen_init_reg(void)
{
    screen_write_byte(0XF9);
    screen_write_word(0x00, 1);
    screen_write_word(0x08, 1);

    screen_write_byte(0xC0);
    screen_write_word(0x19, 1);
    screen_write_word(0x1a, 1);

    screen_write_byte(0xC1);
    screen_write_word(0x45, 1);
    screen_write_word(0x00, 1);

    screen_write_byte(0xC2);
    screen_write_word(0x33, 1);

    screen_write_byte(0XC5);
    screen_write_word(0x00, 1);
    screen_write_word(0x28, 1);

    screen_write_byte(0xB1);
    screen_write_word(0xA0, 1);
    screen_write_word(0x11, 1);

    screen_write_byte(0xB4);
    screen_write_word(0x02, 1);

    screen_write_byte(0xB6);
    screen_write_word(0x00, 1);
    screen_write_word(0x42, 1);
    screen_write_word(0x3B, 1);

    screen_write_byte(0xB7);
    screen_write_word(0x07, 1);

    screen_write_byte(0xE0);
    screen_write_word(0x1F, 1);
    screen_write_word(0x25, 1);
    screen_write_word(0x22, 1);
    screen_write_word(0x0B, 1);
    screen_write_word(0x06, 1);
    screen_write_word(0x0A, 1);
    screen_write_word(0x4E, 1);
    screen_write_word(0xC6, 1);
    screen_write_word(0x39, 1);
    screen_write_word(0x00, 1);
    screen_write_word(0x00, 1);
    screen_write_word(0x00, 1);
    screen_write_word(0x00, 1);
    screen_write_word(0x00, 1);
    screen_write_word(0x00, 1);

    screen_write_byte(0XE1);
    screen_write_word(0x1F, 1);
    screen_write_word(0x3F, 1);
    screen_write_word(0x3F, 1);
    screen_write_word(0x0F, 1);
    screen_write_word(0x1F, 1);
    screen_write_word(0x0F, 1);
    screen_write_word(0x46, 1);
    screen_write_word(0x49, 1);
    screen_write_word(0x31, 1);
    screen_write_word(0x05, 1);
    screen_write_word(0x09, 1);
    screen_write_word(0x03, 1);
    screen_write_word(0x1C, 1);
    screen_write_word(0x1A, 1);
    screen_write_word(0x00, 1);

    screen_write_byte(0XF1);
    screen_write_word(0x36, 1);
    screen_write_word(0x04, 1);
    screen_write_word(0x00, 1);
    screen_write_word(0x3C, 1);
    screen_write_word(0x0F, 1);
    screen_write_word(0x0F, 1);
    screen_write_word(0xA4, 1);
    screen_write_word(0x02, 1);

    screen_write_byte(0XF2);
    screen_write_word(0x18, 1);
    screen_write_word(0xA3, 1);
    screen_write_word(0x12, 1);
    screen_write_word(0x02, 1);
    screen_write_word(0x32, 1);
    screen_write_word(0x12, 1);
    screen_write_word(0xFF, 1);
    screen_write_word(0x32, 1);
    screen_write_word(0x00, 1);

    screen_write_byte(0XF4);
    screen_write_word(0x40, 1);
    screen_write_word(0x00, 1);
    screen_write_word(0x08, 1);
    screen_write_word(0x91, 1);
    screen_write_word(0x04, 1);

    screen_write_byte(0XF8);
    screen_write_word(0x21, 1);
    screen_write_word(0x04, 1);

    screen_write_byte(0X3A);
    screen_write_word(0x55, 1);

}

void screen_gram_scan(LCD_SCAN_DIR Scan_dir)
{

    uint16_t DisFunReg_Data       = 0;
    uint16_t MemoryAccessReg_Data = 0;

    switch (Scan_dir)
    {
    case L2R_U2D:
        MemoryAccessReg_Data = 0x08;
        DisFunReg_Data       = 0x22;
        break;
    case L2R_D2U:
        MemoryAccessReg_Data = 0x08;
        DisFunReg_Data       = 0x62;
        break;
    case R2L_U2D:
        MemoryAccessReg_Data = 0x08;
        DisFunReg_Data       = 0x02;
        break;
    case R2L_D2U:
        MemoryAccessReg_Data = 0x08;
        DisFunReg_Data       = 0x42;
        break;
    case U2D_L2R:
        MemoryAccessReg_Data = 0x28;
        DisFunReg_Data       = 0x22;
        break;
    case U2D_R2L:
        MemoryAccessReg_Data = 0x28;
        DisFunReg_Data       = 0x02;
        break;
    case D2U_L2R:
        MemoryAccessReg_Data = 0x28;
        DisFunReg_Data       = 0x62;
        break;
    case D2U_R2L:
        MemoryAccessReg_Data = 0x28;
        DisFunReg_Data       = 0x42;
        break;
    }

    sLCD_DIS.LCD_Scan_Dir = Scan_dir;

    if(Scan_dir == L2R_U2D || Scan_dir == L2R_D2U || Scan_dir == R2L_U2D || Scan_dir == R2L_D2U)
    {
        sLCD_DIS.LCD_Dis_Column = LCD_HEIGHT;
        sLCD_DIS.LCD_Dis_Page   = LCD_WIDTH;
    }
    else
    {
        sLCD_DIS.LCD_Dis_Column = LCD_WIDTH;
        sLCD_DIS.LCD_Dis_Page   = LCD_HEIGHT;
    }

    screen_write_byte(0xB6);
    screen_write_word(0X00, 1);
    screen_write_word(DisFunReg_Data, 1);

    screen_write_byte(0x36);
    screen_write_word(MemoryAccessReg_Data, 1);
}

void screen_init()
{
    screen_init_reg();
    screen_gram_scan(SCAN_DIR_DFT);
    vTaskDelay(200/portTICK_PERIOD_MS);
    screen_write_byte(0x11);
    vTaskDelay(120/portTICK_PERIOD_MS);
    screen_write_byte(0x29);
    screen_clear(LCD_BACKGROUND);
}

void screen_set_window(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend)
{
    screen_write_byte(0x2A);
    screen_write_word(Xstart >> 8  , 1);
    screen_write_word(Xstart & 0xff, 1);
    screen_write_word((Xend - 1) >> 8  , 1);
    screen_write_word((Xend - 1) & 0xff, 1);

    screen_write_byte(0x2B);
    screen_write_word(Ystart >> 8  , 1);
    screen_write_word(Ystart & 0xff, 1);
    screen_write_word((Yend - 1) >> 8  , 1);
    screen_write_word((Yend - 1) & 0xff, 1);
    screen_write_byte(0x2C);
}

void screen_set_cursor(POINT Xpoint, POINT Ypoint)
{
    screen_set_window(Xpoint, Ypoint, Xpoint, Ypoint);
}

void screen_set_color(COLOR Color, POINT Xpoint, POINT Ypoint)
{
    screen_write_word(Color, (uint32_t)Xpoint * (uint32_t)Ypoint);
}

void screen_set_point_color( POINT Xpoint, POINT Ypoint, COLOR Color)
{
    if((Xpoint <= sLCD_DIS.LCD_Dis_Column) && (Ypoint <= sLCD_DIS.LCD_Dis_Page))
    {
        screen_set_cursor(Xpoint, Ypoint);
        screen_set_color(Color, 1, 1);
    }
}

void screen_set_area_color(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend, COLOR Color)
{
    if((Xend > Xstart) && (Yend > Ystart))
    {
        screen_set_window(Xstart, Ystart, Xend, Yend);
        screen_set_color(Color, Xend - Xstart, Yend - Ystart);
    }
}
void screen_clear(COLOR Color)
{
    screen_set_area_color(0, 0, sLCD_DIS.LCD_Dis_Column, sLCD_DIS.LCD_Dis_Page, Color);
}

void screen_draw_point(POINT Xpoint, POINT Ypoint, COLOR Color,
                   DOT_PIXEL Dot_Pixel, DOT_STYLE DOT_STYLE)
{
    if(Xpoint > sLCD_DIS.LCD_Dis_Column || Ypoint > sLCD_DIS.LCD_Dis_Page)
    {
        ESP_LOGD(TAG, "screen_draw_point Input exceeds the normal display range\r\n");
        return;
    }

    int16_t XDir_Num, YDir_Num;
    if(DOT_STYLE == DOT_STYLE_DFT)
    {
        for(XDir_Num = 0; XDir_Num < 2 * Dot_Pixel - 1; XDir_Num++)
        {
            for(YDir_Num = 0; YDir_Num < 2 * Dot_Pixel - 1; YDir_Num++) screen_set_point_color(Xpoint + XDir_Num - Dot_Pixel + 1, Ypoint + YDir_Num - Dot_Pixel + 1, Color);
        }
    }
    else
    {
        for(XDir_Num = 0; XDir_Num <  Dot_Pixel; XDir_Num++)
        {
            for(YDir_Num = 0; YDir_Num <  Dot_Pixel; YDir_Num++) screen_set_point_color(Xpoint + XDir_Num, Ypoint + YDir_Num, Color);
        }
    }
}

void screen_draw_line(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
                  COLOR Color, LINE_STYLE Line_Style, DOT_PIXEL Dot_Pixel)
{
    if(Xstart > sLCD_DIS.LCD_Dis_Column || Ystart > sLCD_DIS.LCD_Dis_Page ||
        Xend > sLCD_DIS.LCD_Dis_Column || Yend > sLCD_DIS.LCD_Dis_Page)
    {
        ESP_LOGD(TAG, "screen_draw_line Input exceeds the normal display range\r\n");
        return;
    }

    if(Xstart > Xend) screen_swap(Xstart, Xend);
    if(Ystart > Yend) screen_swap(Ystart, Yend);

    POINT Xpoint = Xstart;
    POINT Ypoint = Ystart;
    int dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
    int dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;
    int XAddway = Xstart < Xend ? 1 : -1;
    int YAddway = Ystart < Yend ? 1 : -1;
    int Esp = dx + dy;
    char Line_Style_Temp = 0;

    for(;;)
    {
        Line_Style_Temp++;
        if(Line_Style == LINE_DOTTED && Line_Style_Temp % 3 == 0)
        {
            ESP_LOGD(TAG, "LINE_DOTTED\r\n");
            screen_draw_point(Xpoint, Ypoint, LCD_BACKGROUND, Dot_Pixel, DOT_STYLE_DFT);
            Line_Style_Temp = 0;
        }
        else
        {
            screen_draw_point(Xpoint, Ypoint, Color, Dot_Pixel, DOT_STYLE_DFT);
        }
        if(2 * Esp >= dy)
        {
            if(Xpoint == Xend) break;
            Esp += dy;
            Xpoint += XAddway;
        }
        if(2 * Esp <= dx)
        {
            if(Ypoint == Yend) break;
            Esp += dx;
            Ypoint += YAddway;
        }
    }
}

void screen_draw_rectangle(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,
                       COLOR Color, DRAW_FILL Filled, DOT_PIXEL Dot_Pixel)
{
    if(Xstart > sLCD_DIS.LCD_Dis_Column || Ystart > sLCD_DIS.LCD_Dis_Page ||
      Xend > sLCD_DIS.LCD_Dis_Column || Yend > sLCD_DIS.LCD_Dis_Page)
    {
        ESP_LOGD(TAG, "Input exceeds the normal display range\r\n");
        return;
    }

    if(Xstart > Xend) screen_swap(Xstart, Xend);
    if(Ystart > Yend) screen_swap(Ystart, Yend);

    if(Filled)
    {
        screen_set_area_color(Xstart, Ystart, Xend, Yend, Color);
    }
    else
    {
        screen_draw_line(Xstart, Ystart, Xend, Ystart, Color, LINE_SOLID, Dot_Pixel);
        screen_draw_line(Xstart, Ystart, Xstart, Yend, Color, LINE_SOLID, Dot_Pixel);
        screen_draw_line(Xend, Yend, Xend, Ystart, Color, LINE_SOLID, Dot_Pixel);
        screen_draw_line(Xend, Yend, Xstart, Yend, Color, LINE_SOLID, Dot_Pixel);
    }
}

void screen_draw_circle(POINT X_Center, POINT Y_Center, LENGTH Radius,
                    COLOR Color, DRAW_FILL Draw_Fill, DOT_PIXEL Dot_Pixel)
{
    if(X_Center > sLCD_DIS.LCD_Dis_Column || Y_Center >= sLCD_DIS.LCD_Dis_Page)
    {
        ESP_LOGD(TAG, "screen_draw_circle Input exceeds the normal display range\r\n");
        return;
    }

    int16_t XCurrent, YCurrent;
    XCurrent = 0;
    YCurrent = Radius;
    int16_t Esp = 3 - (Radius << 1);
    int16_t sCountY;
    if(Draw_Fill == DRAW_FULL)
    {
        while (XCurrent <= YCurrent)
        {
            for(sCountY = XCurrent; sCountY <= YCurrent; sCountY++)
            {
                screen_draw_point(X_Center + XCurrent, Y_Center + sCountY , Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                screen_draw_point(X_Center - XCurrent, Y_Center + sCountY , Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                screen_draw_point(X_Center - sCountY , Y_Center + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                screen_draw_point(X_Center - sCountY , Y_Center - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                screen_draw_point(X_Center - XCurrent, Y_Center - sCountY , Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                screen_draw_point(X_Center + XCurrent, Y_Center - sCountY , Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                screen_draw_point(X_Center + sCountY , Y_Center - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
                screen_draw_point(X_Center + sCountY , Y_Center + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
            }
            if(Esp < 0) Esp += 4 * XCurrent + 6;
            else Esp += 10 + 4 * (XCurrent - YCurrent--);
            XCurrent++;
        }
    }
    else
    {
        while(XCurrent <= YCurrent)
        {
            screen_draw_point(X_Center + XCurrent, Y_Center + YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT);
            screen_draw_point(X_Center - XCurrent, Y_Center + YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT);
            screen_draw_point(X_Center - YCurrent, Y_Center + XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT);
            screen_draw_point(X_Center - YCurrent, Y_Center - XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT);
            screen_draw_point(X_Center - XCurrent, Y_Center - YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT);
            screen_draw_point(X_Center + XCurrent, Y_Center - YCurrent, Color, Dot_Pixel, DOT_STYLE_DFT);
            screen_draw_point(X_Center + YCurrent, Y_Center - XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT);
            screen_draw_point(X_Center + YCurrent, Y_Center + XCurrent, Color, Dot_Pixel, DOT_STYLE_DFT);
            if(Esp < 0) Esp += 4 * XCurrent + 6;
            else Esp += 10 + 4 * (XCurrent - YCurrent--);
            XCurrent ++;
        }
    }
}

void screen_print_char(POINT Xpoint, POINT Ypoint, const char Acsii_Char,
                 sFONT *Font, COLOR Color_Background, COLOR Color_Foreground)
{
    POINT Page, Column;
    uint32_t buffer_n = 0;

    if(Xpoint > sLCD_DIS.LCD_Dis_Column || Ypoint > sLCD_DIS.LCD_Dis_Page)
    {
        ESP_LOGD(TAG, "screen_print_char Input exceeds the normal display range\r\n");
        return;
    }

    uint32_t Char_Offset = (Acsii_Char - ' ') * Font->Height * (Font->Width / 8 + (Font->Width % 8 ? 1 : 0));
    const unsigned char *ptr = &Font->table[Char_Offset];
    screen_set_window(Xpoint, Ypoint, Xpoint + Font->Width, Ypoint + Font->Height);
    for(Page = 0; Page < Font->Height; Page++)
    {
        for(Column = 0; Column < Font->Width; Column++)
        {
            if(*ptr & (0x80 >> (Column % 8))) tx_data_16b[buffer_n++] = SPI_SWAP_DATA_TX(Color_Foreground, 16);
            else tx_data_16b[buffer_n++] = SPI_SWAP_DATA_TX(Color_Background, 16);
            if(Column % 8 == 7) ptr++;
            if(buffer_n == SCREEN_BUFFER)
            {
                screen_write_buffer(buffer_n);
                buffer_n = 0;
            }
        }
        if(Font->Width % 8 != 0) ptr++;
    }
    if(buffer_n != 0) screen_write_buffer(buffer_n);
}

void screen_print_transp_char(POINT Xpoint, POINT Ypoint, const char Acsii_Char,
                 sFONT *Font, COLOR Color_Foreground)
{
    POINT Page, Column;

    if(Xpoint > sLCD_DIS.LCD_Dis_Column || Ypoint > sLCD_DIS.LCD_Dis_Page)
    {
        ESP_LOGD(TAG, "screen_print_transp_char Input exceeds the normal display range\r\n");
        return;
    }

    uint32_t Char_Offset = (Acsii_Char - ' ') * Font->Height * (Font->Width / 8 + (Font->Width % 8 ? 1 : 0));
    const unsigned char *ptr = &Font->table[Char_Offset];
    screen_set_window(Xpoint, Ypoint, Xpoint + Font->Width, Ypoint + Font->Height);
    for(Page = 0; Page < Font->Height; Page++)
    {
        for(Column = 0; Column < Font->Width; Column++)
        {
            if(*ptr & (0x80 >> (Column % 8))) screen_set_point_color(Xpoint + Column, Ypoint + Page, Color_Foreground);;
            if(Column % 8 == 7) ptr++;
        }
        if(Font->Width % 8 != 0) ptr++;
    }
}

void screen_print_text(POINT Xstart, POINT Ystart, const char *pString,
                      sFONT *Font, COLOR Color_Background, COLOR Color_Foreground)
{
    POINT Xpoint = Xstart;
    POINT Ypoint = Ystart;

    if(Xstart > sLCD_DIS.LCD_Dis_Column || Ystart > sLCD_DIS.LCD_Dis_Page) {
        ESP_LOGD(TAG, "screen_print_text Input exceeds the normal display range\r\n");
        return;
    }

    while (* pString != '\0')
    {
        if((Xpoint + Font->Width) > sLCD_DIS.LCD_Dis_Column || *pString == '\n')
        {
            Xpoint = Xstart;
            Ypoint += Font->Height;
        }
        if((Ypoint  + Font->Height) > sLCD_DIS.LCD_Dis_Page)
        {
            Xpoint = Xstart;
            Ypoint = Ystart;
        }
        if(*pString != '\n')
        {
            screen_print_char(Xpoint, Ypoint, * pString, Font, Color_Background, Color_Foreground);
            Xpoint += Font->Width;
        }
        pString++;
    }
}

void screen_print_transp_text(POINT Xstart, POINT Ystart, const char *pString,
                      sFONT *Font, COLOR Color_Foreground)
{
    POINT Xpoint = Xstart;
    POINT Ypoint = Ystart;

    if(Xstart > sLCD_DIS.LCD_Dis_Column || Ystart > sLCD_DIS.LCD_Dis_Page) {
        ESP_LOGD(TAG, "screen_print_transp_text Input exceeds the normal display range\r\n");
        return;
    }

    while (* pString != '\0')
    {
        if((Xpoint + Font->Width) > sLCD_DIS.LCD_Dis_Column || *pString == '\n')
        {
            Xpoint = Xstart;
            Ypoint += Font->Height;
        }
        if((Ypoint  + Font->Height) > sLCD_DIS.LCD_Dis_Page)
        {
            Xpoint = Xstart;
            Ypoint = Ystart;
        }
        if(*pString != '\n')
        {
            screen_print_transp_char(Xpoint, Ypoint, * pString, Font, Color_Foreground);
            Xpoint += Font->Width;
        }
        pString++;
    }
}

void screen_draw_from_rom(POINT Xpoint, POINT Ypoint, sFONT *table, COLOR Color_Background)
{
    POINT Page, Column;
    uint32_t buffer_n = 0;

    if(Xpoint > sLCD_DIS.LCD_Dis_Column || Ypoint > sLCD_DIS.LCD_Dis_Page)
    {
        ESP_LOGD(TAG, "screen_logo Input exceeds the normal display range\r\n");
        return;
    }
    const unsigned char *ptr = table->table;

    screen_set_window(Xpoint, Ypoint, Xpoint + table->Width, Ypoint + table->Height);
    for(Page = 0; Page < table->Height; Page++)
    {
        for(Column = 0; Column < table->Width; Column++)
        {
            if(*ptr & (0x80 >> (Column % 8))) tx_data_16b[buffer_n++] = SPI_SWAP_DATA_TX(table->color, 16);
            else tx_data_16b[buffer_n++] = SPI_SWAP_DATA_TX(Color_Background, 16);
            if(Column % 8 == 7) ptr++;
            if(buffer_n == SCREEN_BUFFER)
            {
                screen_write_buffer(buffer_n);
                buffer_n = 0;
            }
        }
        if(table->Width % 8 != 0) ptr++;
    }
    if(buffer_n != 0) screen_write_buffer(buffer_n);
}

static void screen_logo(POINT Xpoint, POINT Ypoint)
{
    POINT Page, Column;
    uint32_t buffer_n = 0;

    if(Xpoint > sLCD_DIS.LCD_Dis_Column || Ypoint > sLCD_DIS.LCD_Dis_Page)
    {
        ESP_LOGD(TAG, "screen_logo Input exceeds the normal display range\r\n");
        return;
    }
    const unsigned char *ptr0 = dreamit_LOGO_Top.table;
    const unsigned char *ptr1 = dreamit_LOGO_Bot.table;

    screen_set_window(Xpoint, Ypoint, Xpoint + dreamit_LOGO_Bot.Width, Ypoint + dreamit_LOGO_Bot.Height);
    for(Page = 0; Page < dreamit_LOGO_Bot.Height; Page++)
    {
        for(Column = 0; Column < dreamit_LOGO_Bot.Width; Column++)
        {
            if     (*ptr0 & (0x80 >> (Column % 8))) tx_data_16b[buffer_n++] = SPI_SWAP_DATA_TX(dreamit_LOGO_Top.color, 16);
            else if(*ptr1 & (0x80 >> (Column % 8))) tx_data_16b[buffer_n++] = SPI_SWAP_DATA_TX(dreamit_LOGO_Bot.color, 16);
            else tx_data_16b[buffer_n++] = SPI_SWAP_DATA_TX(LCD_WHITE, 16);
            if(Column % 8 == 7)
            {
                ptr0++;
                ptr1++;
            }
            if(buffer_n == SCREEN_BUFFER)
            {
                screen_write_buffer(buffer_n);
                buffer_n = 0;
            }
        }
        if(dreamit_LOGO_Bot.Width % 8 != 0)
        {
            ptr0++;
            ptr1++;
        }
    }
    if(buffer_n != 0) screen_write_buffer(buffer_n);
}

static void screen_big_logo(POINT Xpoint, POINT Ypoint)
{
    POINT Page, Column;
    uint32_t buffer_n = 0;

    if(Xpoint > sLCD_DIS.LCD_Dis_Column || Ypoint > sLCD_DIS.LCD_Dis_Page)
    {
        ESP_LOGD(TAG, "screen_logo Input exceeds the normal display range\r\n");
        return;
    }
    const unsigned char *ptr0 = dreamit_LOGO_Big_Top.table;
    const unsigned char *ptr1 = dreamit_LOGO_Big_Bot.table;
    const unsigned char *ptr2 = dreamit_LOGO_Big_Text.table;

    screen_set_window(Xpoint, Ypoint, Xpoint + dreamit_LOGO_Big_Top.Width, Ypoint + dreamit_LOGO_Big_Top.Height);
    for(Page = 0; Page < dreamit_LOGO_Big_Top.Height; Page++)
    {
        for(Column = 0; Column < dreamit_LOGO_Big_Top.Width; Column++)
        {
            if     (*ptr0 & (0x80 >> (Column % 8))) tx_data_16b[buffer_n++] = SPI_SWAP_DATA_TX(dreamit_LOGO_Big_Top.color, 16);
            else if(*ptr1 & (0x80 >> (Column % 8))) tx_data_16b[buffer_n++] = SPI_SWAP_DATA_TX(dreamit_LOGO_Big_Bot.color, 16);
            else if(*ptr2 & (0x80 >> (Column % 8))) tx_data_16b[buffer_n++] = SPI_SWAP_DATA_TX(dreamit_LOGO_Big_Text.color, 16);
            else tx_data_16b[buffer_n++] = SPI_SWAP_DATA_TX(LCD_WHITE, 16);
            if(Column % 8 == 7)
            {
                ptr0++;
                ptr1++;
                ptr2++;
            }
            if(buffer_n == SCREEN_BUFFER)
            {
                screen_write_buffer(buffer_n);
                buffer_n = 0;
            }
        }
        if(dreamit_LOGO_Big_Top.Width % 8 != 0)
        {
            ptr0++;
            ptr1++;
            ptr2++;
        }
    }
    if(buffer_n != 0) screen_write_buffer(buffer_n);
}

static void screen_cross()
{
    screen_clear(LCD_RED);
    screen_draw_from_rom(sLCD_DIS.LCD_Dis_Column/2 - circle_Sign.Width /2,
                         sLCD_DIS.LCD_Dis_Page  /2 - circle_Sign.Height/2 - 40,
                         &circle_Sign, LCD_RED);
    screen_draw_from_rom(sLCD_DIS.LCD_Dis_Column/2 - cross_Sign.Width /2,
                         sLCD_DIS.LCD_Dis_Page  /2 - cross_Sign.Height/2  - 40,
                         &cross_Sign, LCD_WHITE);
    screen_print_text(sLCD_DIS.LCD_Dis_Column/2 - Font24.Width*4,
                      sLCD_DIS.LCD_Dis_Page  /2 - circle_Sign.Height/2 + 180,
                      "DENEGADO", &Font24, LCD_RED, LCD_WHITE);
}

static void screen_check()
{
    screen_clear(LCD_GREEN);
    screen_draw_from_rom(sLCD_DIS.LCD_Dis_Column/2 - circle_Sign.Width /2,
                         sLCD_DIS.LCD_Dis_Page  /2 - circle_Sign.Height/2 - 40,
                         &circle_Sign, LCD_GREEN);
    screen_draw_from_rom(sLCD_DIS.LCD_Dis_Column/2 - check_Sign.Width /2,
                         sLCD_DIS.LCD_Dis_Page  /2 - check_Sign.Height/2  - 40,
                         &check_Sign, LCD_WHITE);
    screen_print_text(sLCD_DIS.LCD_Dis_Column/2 - Font24.Width*4,
                      sLCD_DIS.LCD_Dis_Page  /2 - circle_Sign.Height/2 + 180,
                      "APROBADO", &Font24, LCD_GREEN, LCD_WHITE);
}

void screen_print_conf(char *text, bool def_load)
{
    screen_clear(LCD_BACKGROUND);
    screen_logo(10, 10);
    screen_logo(sLCD_DIS.LCD_Dis_Column - 42, 10);
    screen_print_text(sLCD_DIS.LCD_Dis_Column/2 - Font16.Width*10/2,
                      20,
                      "dreamit.cl",
                      &Font16, FONT_BACKGROUND, FONT_FOREGROUND);
    screen_print_text(10, 65, text, &Font24, FONT_BACKGROUND, FONT_FOREGROUND);
    screen_draw_rectangle(10,  50, sLCD_DIS.LCD_Dis_Column - 10,  56, LCD_LOGO_BOT, DRAW_FULL, DOT_PIXEL_DFT);
    screen_draw_rectangle(10, 194, sLCD_DIS.LCD_Dis_Column - 10, 200, LCD_LOGO_BOT, DRAW_FULL, DOT_PIXEL_DFT);

    screen_print_text(10, 210 + 15,
                      "Set new configuration:",
                      &Font16, FONT_BACKGROUND, FONT_FOREGROUND);
    screen_draw_from_rom(sLCD_DIS.LCD_Dis_Column - button_Sign.Width - 10,
                         210 + 15 - button_Sign.Height/2 + Font16.Height/2,
                         &button_Sign, LCD_WHITE);
    screen_print_transp_text(sLCD_DIS.LCD_Dis_Column - button_Sign.Width/2 - 10 - Font24.Width*3/2,
                             210 + 15 - Font24.Height/2 + Font16.Height/2,
                             "New", &Font24, LCD_WHITE);

    if(def_load)
    {
        screen_print_text(10, 210 + 15 + 10 + button_Sign.Height,
                          "Load default configuration:",
                          &Font16, FONT_BACKGROUND, FONT_FOREGROUND);

        screen_draw_from_rom(sLCD_DIS.LCD_Dis_Column - button_Sign.Width - 10,
                             210 + 15 + 10 + button_Sign.Height/2 + Font16.Height/2,
                             &button_Sign, LCD_WHITE);
        screen_print_transp_text(sLCD_DIS.LCD_Dis_Column - button_Sign.Width/2 - 10 - Font24.Width*4/2,
                                 210 + 15 + 10 + button_Sign.Height + Font16.Height/2 - Font24.Height/2,
                                 "Load", &Font24, LCD_WHITE);
    }
}

static void screen_interface()
{
    screen_clear(LCD_BACKGROUND);
    screen_big_logo((QR_SIZE*21 + 2*QR_OFFSET + sLCD_DIS.LCD_Dis_Column - dreamit_LOGO_Big_Top.Width)/2, QR_OFFSET);
    screen_draw_rectangle(               QR_OFFSET/2,
                                         QR_OFFSET/2,
                          QR_SIZE*21 + 3*QR_OFFSET/2,
                          QR_SIZE*21 + 3*QR_OFFSET/2,
                          LCD_LOGO_BOT, DRAW_EMPTY, DOT_PIXEL_DFT);

    screen_draw_from_rom((sLCD_DIS.LCD_Dis_Column + QR_SIZE*21 + 2*QR_OFFSET)/2 - button_Sign.Width/2,
                         2*QR_OFFSET + dreamit_LOGO_Big_Top.Height + 20,
                         &button_Sign, LCD_WHITE);

    screen_print_transp_text((sLCD_DIS.LCD_Dis_Column + QR_SIZE*21 + 2*QR_OFFSET)/2 - Font20.Width*7/2,
                             2*QR_OFFSET + dreamit_LOGO_Big_Top.Height + 20 + button_Sign.Height/2 - Font20.Height/2,
                             "Reserva", &Font20, LCD_WHITE);

    screen_draw_from_rom((sLCD_DIS.LCD_Dis_Column + QR_SIZE*21 + 2*QR_OFFSET)/2 - button_Sign.Width/2,
                         2*QR_OFFSET + dreamit_LOGO_Big_Top.Height + 80,
                         &button_Sign, LCD_WHITE);

    screen_print_transp_text((sLCD_DIS.LCD_Dis_Column + QR_SIZE*21 + 2*QR_OFFSET)/2 - Font20.Width*6/2,
                             2*QR_OFFSET + dreamit_LOGO_Big_Top.Height + 80 + button_Sign.Height/2 - Font20.Height/2,
                             "Cedula", &Font20, LCD_WHITE);

    touch_set_context("", TOUCH_QR_CODE);
}

void screen_draw_keyboard(const char *keyboard_str_in)
{
    char *keyboard_str = (char*)malloc(41);
    strncpy(keyboard_str, keyboard_str_in, 41);
    for(int j = 0; j < 4;j++)
    {
        for(int i = 0; i < 10;i++)
        {
            screen_draw_rectangle(QR_OFFSET +     i*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/10,
                                  sLCD_DIS.LCD_Dis_Page/2 +     j*40 - 20,
                                  QR_OFFSET + (i+1)*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/10,
                                  sLCD_DIS.LCD_Dis_Page/2 + (j+1)*40 - 20,
                                  LCD_GRAY, DRAW_EMPTY, DOT_PIXEL_DFT);
            screen_print_char(QR_OFFSET + (2*i+1)*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/(2*10) - Font24.Width/2,
                              sLCD_DIS.LCD_Dis_Page/2 + (2*j+1)*40/2 - 20 - Font24.Height/2,
                              *(keyboard_str++),
                              &Font24,
                              LCD_WHITE,
                              LCD_BLACK);
        }
    }
}

void screen_draw_input_reservation(void)
{
    screen_clear(LCD_BACKGROUND);
    screen_draw_keyboard("1234567890QWERTYUIOPASDFGHJKL< ZXCVBNM >");
}

void screen_draw_input_interface(void)
{
    screen_clear(LCD_BACKGROUND);
    const char *keyboard_str = "123456789<0>";
    for(int j = 0; j < 4;j++)
    {
        for(int i = 0; i < 3;i++)
        {
            screen_draw_rectangle(QR_OFFSET +     i*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/3,
                                  sLCD_DIS.LCD_Dis_Page/2 +     j*40 - 20,
                                  QR_OFFSET + (i+1)*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/3,
                                  sLCD_DIS.LCD_Dis_Page/2 + (j+1)*40 - 20,
                                  LCD_GRAY, DRAW_EMPTY, DOT_PIXEL_DFT);
            screen_print_char(QR_OFFSET + (2*i+1)*(sLCD_DIS.LCD_Dis_Column - 2*QR_OFFSET)/6 - Font24.Width/2,
                              sLCD_DIS.LCD_Dis_Page/2 + (2*j+1)*40/2 - 20 - Font24.Height/2,
                              *(keyboard_str++),
                              &Font24,
                              LCD_WHITE,
                              LCD_BLACK);
        }
    }
}

static void screen_draw_qr(uint8_t *qrcode)
{
    int size = qrcodegen_getSize(qrcode);
    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < size; j++)
        {
            if(qrcodegen_getModule(qrcode, i, j))
                screen_draw_rectangle(i*QR_SIZE + QR_OFFSET,
                                      j*QR_SIZE + QR_OFFSET,
                                  (i+1)*QR_SIZE + QR_OFFSET,
                                  (j+1)*QR_SIZE + QR_OFFSET,
                                  LCD_BLACK, DRAW_FULL, DOT_PIXEL_DFT);
            else
                screen_draw_rectangle(i*QR_SIZE + QR_OFFSET,
                                      j*QR_SIZE + QR_OFFSET,
                                  (i+1)*QR_SIZE + QR_OFFSET,
                                  (j+1)*QR_SIZE + QR_OFFSET,
                                     LCD_WHITE, DRAW_FULL, DOT_PIXEL_DFT);
        }
    }
}

void screen_task(void *arg)
{
    enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;
    uint8_t qrcode[qrcodegen_BUFFER_LEN_FOR_VERSION(2)];
    uint8_t temp_buffer[qrcodegen_BUFFER_LEN_FOR_VERSION(2)];
    screen_interface();
    while(1)
    {
        if(xQueueReceive(screen_task_queue, &screen_task_data, portMAX_DELAY))
        {
            if(screen_task_data.status == 0)
            {
                if(touch_context_status == TOUCH_QR_CODE)
                {
                    qrcodegen_encodeText(screen_task_data.msg,
                                         temp_buffer,
                                         qrcode,
                                         errCorLvl,
                                         qrcodegen_VERSION_MIN,
                                         qrcodegen_VERSION_MAX,
                                         qrcodegen_Mask_AUTO,
                                         true);
                    screen_draw_qr(qrcode);
                }
            }
            else if(screen_task_data.status == 1)
            {
                if(strcmp(screen_task_data.msg, "GOOD") == 0) screen_check();
                else if(strcmp(screen_task_data.msg, "BAD") == 0) screen_cross();
                vTaskDelay(screen_task_data.timer*1000/portTICK_PERIOD_MS);
                if(touch_context_status == TOUCH_QR_CODE)
                {
                    screen_interface();
                    screen_draw_qr(qrcode);
                }
                if(touch_context_status == TOUCH_INPUT_RESERVATION)
                {
                    screen_draw_input_reservation();
                }
                if(touch_context_status == TOUCH_INPUT_RUT)
                {
                    screen_draw_input_interface();
                }
            }
            else if(screen_task_data.status == 2)
            {
                if(touch_context_status == TOUCH_INPUT_RESERVATION)
                {
                    screen_draw_input_reservation();
                }
                if(touch_context_status == TOUCH_INPUT_RUT)
                {
                    screen_draw_input_interface();
                }
            }
        }
    }
}
