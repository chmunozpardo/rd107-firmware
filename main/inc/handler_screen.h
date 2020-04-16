#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

void screen_init();
void screen_gram_scan(LCD_SCAN_DIR Scan_dir);
void screen_clear(COLOR  Color);

void LCD_SetWindow(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend);
void LCD_SetCursor(POINT Xpoint, POINT Ypoint);
void LCD_SetColor(COLOR Color ,POINT Xpoint, POINT Ypoint);
void LCD_SetPointlColor(POINT Xpoint, POINT Ypoint, COLOR Color);
void LCD_SetArealColor(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,COLOR  Color);

#ifdef __cplusplus
}
#endif