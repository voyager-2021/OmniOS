#include "cmd_helpers.h"

extern char g_hostname[32];

void cmd_whoami(ShellCmd *cl)
{
    (void)cl;
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  root@%s\n", g_hostname);
    COLOR_RESET();
}