#include "cmd_helpers.h"

char g_hostname[32] = "omnios";

void cmd_hostname(ShellCmd *cl)
{
    if (cl->argc >= 2) {
        int i = 0;
        const char *s = cl->argv[1];
        while (*s && i < 30) g_hostname[i++] = *s++;
        g_hostname[i] = '\0';
        COLOR_SET(VGA_COL_SUCCESS);
        printf("  Hostname set to: %s\n", g_hostname);
        COLOR_RESET();
    } else {
        COLOR_SET(VGA_COL_INFO);
        printf("  Hostname: %s\n", g_hostname);
        COLOR_RESET();
    }
}