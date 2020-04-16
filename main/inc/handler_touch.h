#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

#include "handler_screen.h"
#include "Waveshare_ILI9486.h"

#define TP_PRESS_DOWN   0x80
#define TP_PRESSED      0x40

void TP_GetAdFac(void);
void TP_Adjust(void);
void TP_Dialog(void);
void TP_DrawBoard(void);
void TP_Init( LCD_SCAN_DIR Lcd_ScanDir );

#ifdef __cplusplus
extern "C" {
#endif