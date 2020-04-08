#include "handler_spi.h"
#include "handler_screen.h"

LCD_DIS sLCD_DIS;

/*******************************************************************************
function:
        Common register initialization
*******************************************************************************/
static void screen_init_reg(void)
{
    screen_write_byte(0XF9);
    screen_write_word(0x00, 1);
    screen_write_word(0x08, 1);

    screen_write_byte(0xC0);
    screen_write_word(0x19, 1);//VREG1OUT POSITIVE
    screen_write_word(0x1a, 1);//VREG2OUT NEGATIVE

    screen_write_byte(0xC1);
    screen_write_word(0x45, 1);//VGH,VGL    VGH>=14V.
    screen_write_word(0x00, 1);

    screen_write_byte(0xC2);//Normal mode, increase can change the display quality, while increasing power consumption
    screen_write_word(0x33, 1);

    screen_write_byte(0XC5);
    screen_write_word(0x00, 1);
    screen_write_word(0x28, 1);//VCM_REG[7:0]. <=0X80.

    screen_write_byte(0xB1);//Sets the frame frequency of full color normal mode
    screen_write_word(0xA0, 1);//0XB0 =70HZ, <=0XB0.0xA0=62HZ
    screen_write_word(0x11, 1);

    screen_write_byte(0xB4);
    screen_write_word(0x02, 1); //2 DOT FRAME MODE,F<=70HZ.

    screen_write_byte(0xB6);//
    screen_write_word(0x00, 1);
    screen_write_word(0x42, 1);//0 GS SS SM ISC[3:0];
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

    screen_write_byte(0X3A);//Set Interface Pixel Format
    screen_write_word(0x55, 1);

}

/********************************************************************************
function:    Set the display scan and color transfer modes
parameter:
        Scan_dir   :   Scan direction
        Colorchose :   RGB or GBR color format
********************************************************************************/
void screen_gram_scan(LCD_SCAN_DIR Scan_dir)
{
    uint16_t MemoryAccessReg_Data = 0; //addr:0x36
    uint16_t DisFunReg_Data = 0; //addr:0xB6

    // Gets the scan direction of GRAM
    switch (Scan_dir)
    {
    case L2R_U2D:                       // Rotation=        0
        MemoryAccessReg_Data = 0x08;    // MADCTL:          BGR
        DisFunReg_Data = 0x22;          // DISPFUNCCTL:     ISC='b0010, SM=0, SS=1, GS=0
        break;
    case L2R_D2U:
        MemoryAccessReg_Data = 0x08;    // MADCTL:          BGR
        DisFunReg_Data = 0x62;
        break;
    case R2L_U2D: //0X4
        MemoryAccessReg_Data = 0x08;    // MADCTL:          BGR
        DisFunReg_Data = 0x02;
        break;
    case R2L_D2U: //0XC                 // Rotation=        2
        MemoryAccessReg_Data = 0x08;    // MADCTL:          BGR
        DisFunReg_Data = 0x42;          // DISPFUNCCTL:     ISC='b0010, SM=0, SS=0, GS=1
        break;
    case U2D_L2R: //0X2
        MemoryAccessReg_Data = 0x28;    // MADCTL:          BGR, MV
        DisFunReg_Data = 0x22;
        break;
    case U2D_R2L: //0X6                 // Rotation=        1
        MemoryAccessReg_Data = 0x28;    // MADCTL:          BGR, MV
        DisFunReg_Data = 0x02;          // DISPFUNCCTL:     ISC='b0010, SM=0, SS=0, GS=0
        break;
    case D2U_L2R: //0XA                 // Rotation=        3
        MemoryAccessReg_Data = 0x28;    // MADCTL:          BGR, MV
        DisFunReg_Data = 0x62;          // DISPFUNCCTRL:    ISC='b0010, SM=0, SS=1, GS=1
        break;
    case D2U_R2L: //0XE
        MemoryAccessReg_Data = 0x28;    // MADCTL: BGR | MV
        DisFunReg_Data = 0x42;
        break;
    }

    //Get the screen scan direction
    sLCD_DIS.LCD_Scan_Dir = Scan_dir;

    //Get GRAM and LCD width and height
    if(Scan_dir == L2R_U2D || Scan_dir == L2R_D2U || Scan_dir == R2L_U2D || Scan_dir == R2L_D2U)
    {
        sLCD_DIS.LCD_Dis_Column    = LCD_HEIGHT;
        sLCD_DIS.LCD_Dis_Page = LCD_WIDTH;
    }
    else
    {
        sLCD_DIS.LCD_Dis_Column    = LCD_WIDTH;
        sLCD_DIS.LCD_Dis_Page = LCD_HEIGHT;
    }

    // Set the read / write scan direction of the frame memory
    screen_write_byte(0xB6);
    screen_write_word(0X00, 1);
    screen_write_word(DisFunReg_Data, 1);

    screen_write_byte(0x36);
    screen_write_word(MemoryAccessReg_Data, 1);
}

/********************************************************************************
function:
    initialization
********************************************************************************/
void screen_init()
{
    //Set the initialization register
    screen_init_reg();

    //Set the display scan and color transfer modes
    screen_gram_scan(SCAN_DIR_DFT);
    vTaskDelay(200/portTICK_PERIOD_MS);

    //sleep out
    screen_write_byte(0x11);
    vTaskDelay(120/portTICK_PERIOD_MS);

    //Turn on the LCD display
    screen_write_byte(0x29);

    screen_clear(WHITE);
}

/********************************************************************************
function:    Sets the start position and size of the display area
parameter:
    Xstart     :   X direction Start coordinates
    Ystart  :   Y direction Start coordinates
    Xend    :   X direction end coordinates
    Yend    :   Y direction end coordinates
********************************************************************************/
void LCD_SetWindow(POINT Xstart, POINT Ystart,    POINT Xend, POINT Yend)
{
    //set the X coordinates
    screen_write_byte(0x2A);
    screen_write_word(Xstart >> 8, 1);        //Set the horizontal starting point to the high octet
    screen_write_word(Xstart & 0xff, 1);      //Set the horizontal starting point to the low octet
    screen_write_word((Xend - 1) >> 8, 1);    //Set the horizontal end to the high octet
    screen_write_word((Xend - 1) & 0xff, 1);  //Set the horizontal end to the low octet

    //set the Y coordinates
    screen_write_byte(0x2B);
    screen_write_word(Ystart >> 8, 1);
    screen_write_word(Ystart & 0xff, 1);
    screen_write_word((Yend - 1) >> 8, 1);
    screen_write_word((Yend - 1) & 0xff, 1);
    screen_write_byte(0x2C);
}

/********************************************************************************
function:    Set the display point (Xpoint, Ypoint)
parameter:
    xStart :   X direction Start coordinates
    xEnd   :   X direction end coordinates
********************************************************************************/
void LCD_SetCursor(POINT Xpoint, POINT Ypoint)
{
    LCD_SetWindow(Xpoint, Ypoint, Xpoint, Ypoint);
}

/********************************************************************************
function:    Set show color
parameter:
        Color  :   Set show color,16-bit depth
********************************************************************************/
//static void LCD_SetColor(LENGTH Dis_Width, LENGTH Dis_Height, COLOR Color ){
void LCD_SetColor(COLOR Color , POINT Xpoint, POINT Ypoint)
{
    screen_write_word(Color , (uint32_t)Xpoint * (uint32_t)Ypoint);
}

/********************************************************************************
function:    Point (Xpoint, Ypoint) Fill the color
parameter:
    Xpoint :   The x coordinate of the point
    Ypoint :   The y coordinate of the point
    Color  :   Set the color
********************************************************************************/
void LCD_SetPointlColor( POINT Xpoint, POINT Ypoint, COLOR Color)
{
    if ((Xpoint <= sLCD_DIS.LCD_Dis_Column) && (Ypoint <= sLCD_DIS.LCD_Dis_Page))
    {
        LCD_SetCursor (Xpoint, Ypoint);
        LCD_SetColor(Color, 1, 1);
    }
}

/********************************************************************************
function:    Fill the area with the color
parameter:
    Xstart :   Start point x coordinate
    Ystart :   Start point y coordinate
    Xend   :   End point coordinates
    Yend   :   End point coordinates
    Color  :   Set the color
********************************************************************************/
void LCD_SetArealColor(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend, COLOR Color)
{
    if((Xend > Xstart) && (Yend > Ystart))
    {
        LCD_SetWindow(Xstart, Ystart, Xend, Yend);
        LCD_SetColor (Color, Xend - Xstart, Yend - Ystart);
    }
}

/********************************************************************************
function:
            Clear screen
********************************************************************************/
void screen_clear(COLOR  Color)
{
    LCD_SetArealColor(0, 0, sLCD_DIS.LCD_Dis_Column, sLCD_DIS.LCD_Dis_Page, Color);
}

void lcd_draw_rectangle(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend, COLOR Color)
{
    screen_write_byte(0x2A);
    screen_write_word(Xstart, 1);
    screen_write_word(Xend - 1, 1);

    screen_write_byte(0x2B);
    screen_write_word(Ystart, 1);
    screen_write_word(Yend - 1, 1);

    screen_write_byte(0x2C);
    screen_write_word(Color , (uint32_t)(Xend - Xstart) * (uint32_t)(Yend - Ystart));
}