#pragma once
#include <stdint.h>
#include <stdbool.h>

/* ============================================================
   OmniOS - VGA Text Mode Driver Header
   80x25 colour text mode, 16 fg / 16 bg colours
   ============================================================ */

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  ((volatile uint16_t *)0xB8000)

typedef enum {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW        = 14,
    VGA_COLOR_WHITE         = 15,
} VGA_Color;

#define VGA_MAKE_COLOR(fg, bg)  ((uint8_t)(((bg) << 4) | ((fg) & 0x0F)))

#define VGA_COL_NORMAL   VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREY,  VGA_COLOR_BLACK)
#define VGA_COL_PROMPT   VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK)
#define VGA_COL_ERROR    VGA_MAKE_COLOR(VGA_COLOR_LIGHT_RED,   VGA_COLOR_BLACK)
#define VGA_COL_SUCCESS  VGA_MAKE_COLOR(VGA_COLOR_LIGHT_CYAN,  VGA_COLOR_BLACK)
#define VGA_COL_HEADER   VGA_MAKE_COLOR(VGA_COLOR_YELLOW,      VGA_COLOR_BLACK)
#define VGA_COL_INFO     VGA_MAKE_COLOR(VGA_COLOR_CYAN,        VGA_COLOR_BLACK)
#define VGA_COL_WARNING  VGA_MAKE_COLOR(VGA_COLOR_YELLOW,      VGA_COLOR_BLACK)
#define VGA_COL_WHITE    VGA_MAKE_COLOR(VGA_COLOR_WHITE,       VGA_COLOR_BLACK)
#define VGA_COL_INPUT    VGA_MAKE_COLOR(VGA_COLOR_WHITE,       VGA_COLOR_BLACK)

void    VGA_Initialize(void);
void    VGA_ClearScreen(void);
void    VGA_SetColor(uint8_t color);
uint8_t VGA_GetColor(void);
void    VGA_PutChar(char c);
void    VGA_PutCharAt(char c, uint8_t color, int x, int y);
void    VGA_WriteString(const char *s);
void    VGA_SetCursorPos(int x, int y);
void    VGA_GetCursorPos(int *x, int *y);
void    VGA_EnableCursor(uint8_t start_scanline, uint8_t end_scanline);
void    VGA_DisableCursor(void);
void    VGA_ScrollUp(int lines);
void    VGA_DrawHLine(char ch, int width, uint8_t color);
void    VGA_SaveState(void);
void    VGA_RestoreState(void);