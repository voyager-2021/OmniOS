/* ============================================================
   OmniOS - VGA Text Mode Driver
   Supports: colour, scrolling, hardware cursor, backspace,
             tabs, carriage return, screen save/restore
   ============================================================ */
#include "vga_text.h"
#include "io.h"
#include <stddef.h>
#include <stdint.h>

#define VGA_CTRL_REG   0x3D4
#define VGA_DATA_REG   0x3D5

static int      s_cursor_x   = 0;
static int      s_cursor_y   = 0;
static uint8_t  s_color      = VGA_COL_NORMAL;

static int      s_saved_x    = 0;
static int      s_saved_y    = 0;
static uint8_t  s_saved_col  = VGA_COL_NORMAL;

static inline uint16_t make_cell(char c, uint8_t color)
{
    return (uint16_t)((uint8_t)c) | ((uint16_t)color << 8);
}

static void hw_cursor_update(void)
{
    uint16_t pos = (uint16_t)(s_cursor_y * VGA_WIDTH + s_cursor_x);
    i686_outb(VGA_CTRL_REG, 0x0F);
    i686_outb(VGA_DATA_REG, (uint8_t)(pos & 0xFF));
    i686_outb(VGA_CTRL_REG, 0x0E);
    i686_outb(VGA_DATA_REG, (uint8_t)((pos >> 8) & 0xFF));
}

void VGA_EnableCursor(uint8_t start_scanline, uint8_t end_scanline)
{
    i686_outb(VGA_CTRL_REG, 0x0A);
    i686_outb(VGA_DATA_REG, (i686_inb(VGA_DATA_REG) & 0xC0) | start_scanline);
    i686_outb(VGA_CTRL_REG, 0x0B);
    i686_outb(VGA_DATA_REG, (i686_inb(VGA_DATA_REG) & 0xE0) | end_scanline);
}

void VGA_DisableCursor(void)
{
    i686_outb(VGA_CTRL_REG, 0x0A);
    i686_outb(VGA_DATA_REG, 0x20);
}

void VGA_ScrollUp(int lines)
{
    if (lines <= 0) return;
    if (lines >= VGA_HEIGHT) { VGA_ClearScreen(); return; }

    for (int y = 0; y < VGA_HEIGHT - lines; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            VGA_MEMORY[y * VGA_WIDTH + x] =
                VGA_MEMORY[(y + lines) * VGA_WIDTH + x];

    for (int y = VGA_HEIGHT - lines; y < VGA_HEIGHT; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            VGA_MEMORY[y * VGA_WIDTH + x] = make_cell(' ', s_color);
}

void VGA_Initialize(void)
{
    s_cursor_x  = 0;
    s_cursor_y  = 0;
    s_color     = VGA_COL_NORMAL;
    VGA_EnableCursor(14, 15);
    VGA_ClearScreen();
}

void VGA_ClearScreen(void)
{
    uint16_t blank = make_cell(' ', s_color);
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        VGA_MEMORY[i] = blank;
    s_cursor_x = 0;
    s_cursor_y = 0;
    hw_cursor_update();
}

void VGA_SetColor(uint8_t color)
{
    s_color = color;
}

uint8_t VGA_GetColor(void)
{
    return s_color;
}

void VGA_SetCursorPos(int x, int y)
{
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= VGA_WIDTH)  x = VGA_WIDTH  - 1;
    if (y >= VGA_HEIGHT) y = VGA_HEIGHT - 1;
    s_cursor_x = x;
    s_cursor_y = y;
    hw_cursor_update();
}

void VGA_GetCursorPos(int *x, int *y)
{
    if (x) *x = s_cursor_x;
    if (y) *y = s_cursor_y;
}

void VGA_PutCharAt(char c, uint8_t color, int x, int y)
{
    if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT) return;
    VGA_MEMORY[y * VGA_WIDTH + x] = make_cell(c, color);
}

void VGA_PutChar(char c)
{
    switch (c) {
        case '\n':
            s_cursor_x = 0;
            s_cursor_y++;
            break;

        case '\r':
            s_cursor_x = 0;
            break;

        case '\t':
            s_cursor_x = (s_cursor_x + 8) & ~7;
            if (s_cursor_x >= VGA_WIDTH) {
                s_cursor_x = 0;
                s_cursor_y++;
            }
            break;

        case '\b':
            if (s_cursor_x > 0) {
                s_cursor_x--;
            } else if (s_cursor_y > 0) {
                s_cursor_y--;
                s_cursor_x = VGA_WIDTH - 1;
            }
            VGA_MEMORY[s_cursor_y * VGA_WIDTH + s_cursor_x] =
                make_cell(' ', s_color);
            break;

        default:
            if (s_cursor_x >= VGA_WIDTH) {
                s_cursor_x = 0;
                s_cursor_y++;
            }
            if ((unsigned char)c >= 0x20) {
                VGA_MEMORY[s_cursor_y * VGA_WIDTH + s_cursor_x] =
                    make_cell(c, s_color);
                s_cursor_x++;
            }
            break;
    }

    if (s_cursor_y >= VGA_HEIGHT) {
        VGA_ScrollUp(1);
        s_cursor_y = VGA_HEIGHT - 1;
        s_cursor_x = 0;
    }

    hw_cursor_update();
}

void VGA_WriteString(const char *s)
{
    if (!s) return;
    while (*s) VGA_PutChar(*s++);
}

void VGA_DrawHLine(char ch, int width, uint8_t color)
{
    uint8_t old = s_color;
    s_color = color;
    for (int i = 0; i < width; i++) VGA_PutChar(ch);
    VGA_PutChar('\n');
    s_color = old;
}

void VGA_SaveState(void)
{
    s_saved_x   = s_cursor_x;
    s_saved_y   = s_cursor_y;
    s_saved_col = s_color;
}

void VGA_RestoreState(void)
{
    s_cursor_x = s_saved_x;
    s_cursor_y = s_saved_y;
    s_color    = s_saved_col;
    hw_cursor_update();
}