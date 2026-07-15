/* ============================================================
   OmniOS - Kernel Debug / Logging
   Direct VGA output. No VFS, no fputs/fprintf/fputc.
   ============================================================ */
#include "debug.h"
#include "stdio.h"
#include "arch/i686/vga_text.h"
#include <stdarg.h>

static const uint8_t g_LevelColors[] = {
    [LVL_DEBUG]    = VGA_MAKE_COLOR(VGA_COLOR_DARK_GREY,   VGA_COLOR_BLACK),
    [LVL_INFO]     = VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREY,  VGA_COLOR_BLACK),
    [LVL_WARN]     = VGA_MAKE_COLOR(VGA_COLOR_YELLOW,      VGA_COLOR_BLACK),
    [LVL_ERROR]    = VGA_MAKE_COLOR(VGA_COLOR_LIGHT_RED,   VGA_COLOR_BLACK),
    [LVL_CRITICAL] = VGA_MAKE_COLOR(VGA_COLOR_WHITE,       VGA_COLOR_RED),
};

static const char *const g_LevelNames[] = {
    [LVL_DEBUG]    = "DEBUG",
    [LVL_INFO]     = "INFO ",
    [LVL_WARN]     = "WARN ",
    [LVL_ERROR]    = "ERROR",
    [LVL_CRITICAL] = "CRIT ",
};

void logf(const char *module, DebugLevel level, const char *fmt, ...)
{
    if (level < MIN_LOG_LEVEL)
        return;

    va_list args;
    va_start(args, fmt);

    uint8_t old_color = VGA_GetColor();

    VGA_SetColor(g_LevelColors[level]);
    puts("[");
    puts(g_LevelNames[level]);
    puts("][");
    puts(module ? module : "?");
    puts("] ");

    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    vprintf(fmt, args);
    putc('\n');

    VGA_SetColor(old_color);

    va_end(args);
}