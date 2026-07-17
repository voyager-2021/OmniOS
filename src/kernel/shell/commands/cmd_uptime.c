#include "cmd_helpers.h"

void cmd_uptime(ShellCmd *cl)
{
    (void)cl;
    uint32_t ticks = g_tick_count;
    uint32_t secs  = ticks / 18;
    uint32_t mins  = secs / 60;
    uint32_t hrs   = mins / 60;
    COLOR_SET(VGA_COL_INFO);
    printf("  Uptime: %02u:%02u:%02u (%u ticks)\n",
           hrs, mins % 60, secs % 60, ticks);
    COLOR_RESET();
}