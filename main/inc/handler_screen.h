#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

void screen_init();
void screen_clear(COLOR Color);
void screen_task(void *arg);
void screen_gram_scan(LCD_SCAN_DIR Scan_dir);

void screen_draw_point(POINT Xpoint, POINT Ypoint, COLOR Color, DOT_PIXEL Dot_Pixel, DOT_STYLE Dot_FillWay);
void screen_draw_line(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend, COLOR Color, LINE_STYLE Line_Style, DOT_PIXEL Dot_Pixel);
void screen_draw_circle(POINT X_Center, POINT Y_Center, LENGTH Radius, COLOR Color, DRAW_FILL Draw_Fill, DOT_PIXEL Dot_Pixel);
void screen_draw_rectangle(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend, COLOR Color, DRAW_FILL Filled, DOT_PIXEL Dot_Pixel);

void screen_print_char(POINT Xstart, POINT Ystart, const char Acsii_Char, sFONT *Font, COLOR Color_Background, COLOR Color_Foreground);
void screen_print_text(POINT Xstart, POINT Ystart, const char * pString, sFONT *Font, COLOR Color_Background, COLOR Color_Foreground);
void screen_print_conf(char *text);

void screen_cross(POINT Xpoint, POINT Ypoint);
void screen_check(POINT Xpoint, POINT Ypoint);

void screen_set_color(COLOR Color ,POINT Xpoint, POINT Ypoint);
void screen_set_cursor(POINT Xpoint, POINT Ypoint);
void screen_set_window(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend);
void screen_set_area_color(POINT Xstart, POINT Ystart, POINT Xend, POINT Yend,COLOR Color);
void screen_set_point_color(POINT Xpoint, POINT Ypoint, COLOR Color);

#ifdef __cplusplus
}
#endif