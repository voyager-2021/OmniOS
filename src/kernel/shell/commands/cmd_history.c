#include "cmd_helpers.h"

void cmd_history(ShellCmd *cl)
{
    (void)cl;
    cprintln("  Command History:", VGA_COL_HEADER);
    if (s_hist_count == 0) { cprintln("  (empty)", VGA_COL_WARNING); return; }
    for (int i = s_hist_count; i >= 1; i--) {
        const char *h = history_get(i);
        if (!h) continue;
        COLOR_SET(VGA_COL_INFO);
        printf("  %3d  ", s_hist_count - i + 1);
        COLOR_RESET();
        puts(h); putc('\n');
    }
}