#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

void screen_printf(char *text);
void qr_task(void *arg);

void GUI_Clear(COLOR Color);
void GUI_DrawPoint(POINT Xpoint, POINT Ypoint, COLOR Color, DOT_PIXEL Dot_Pixel, DOT_STYLE Dot_FillWay);
void GUI_DrawLine(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend, COLOR Color, LINE_STYLE Line_Style, DOT_PIXEL Dot_Pixel);
void GUI_DrawRectangle(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend, COLOR Color, DRAW_FILL Filled , DOT_PIXEL Dot_Pixel );
void GUI_DrawCircle(POINT X_Center, POINT Y_Center, LENGTH Radius, COLOR Color, DRAW_FILL Draw_Fill , DOT_PIXEL Dot_Pixel );
void GUI_Disbitmap(POINT Xpoint, POINT Ypoint, const unsigned char *pMap, POINT Width, POINT Height);
void GUI_DisGrayMap(POINT Xpoint, POINT Ypoint, const unsigned char *pBmp);
void GUI_DisChar(POINT Xstart, POINT Ystart, const char Acsii_Char, sFONT *Font, COLOR Color_Background, COLOR Color_Foreground);
void GUI_DisString_EN(POINT Xstart, POINT Ystart, const char * pString, sFONT *Font, COLOR Color_Background, COLOR Color_Foreground );
void GUI_DisNum(POINT Xpoint, POINT Ypoint, int32_t Nummber, sFONT *Font, COLOR Color_Background, COLOR Color_Foreground );

#ifdef __cplusplus
}
#endif