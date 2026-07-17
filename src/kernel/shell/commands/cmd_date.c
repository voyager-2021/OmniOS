#include "cmd_helpers.h"

void cmd_date(ShellCmd *cl)
{
    (void)cl;
    uint32_t secs = g_tick_count / 18, mins = secs / 60, hrs = mins / 60;
    COLOR_SET(VGA_COL_INFO);
    printf("  System time: %02u:%02u:%02u (since boot)\n", hrs, mins % 60, secs % 60);
    COLOR_RESET();
    cprintln("  Note: No RTC driver. Time relative to boot.", VGA_COL_WARNING);
}