#include "cmd_helpers.h"

void cmd_ascii(ShellCmd *cl)
{
    (void)cl;
    cprintln("  ASCII Table (32-126):", VGA_COL_HEADER);
    int col = 0;
    for (int i = 32; i <= 126; i++) {
        COLOR_SET(VGA_COL_INFO); printf(" %3d", i);
        COLOR_RESET(); printf(" %c  ", (char)i);
        col++;
        if (col % 10 == 0) putc('\n');
    }
    if (col % 10 != 0) putc('\n');
}