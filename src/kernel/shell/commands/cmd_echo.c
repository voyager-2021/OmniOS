#include "cmd_helpers.h"

void cmd_echo(ShellCmd *cl)
{
    for (int i = 1; i < cl->argc; i++) {
        puts(cl->argv[i]);
        if (i < cl->argc - 1) putc(' ');
    }
    putc('\n');
}