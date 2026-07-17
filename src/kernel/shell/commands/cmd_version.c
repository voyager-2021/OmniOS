#include "cmd_helpers.h"

void cmd_version(ShellCmd *cl)
{
    (void)cl;
    cprint("  OmniOS v", VGA_COL_INFO);
    cprint(OMNIOS_VERSION, VGA_COL_SUCCESS);
    cprint("  build ", VGA_COL_INFO);
    cprintln(OMNIOS_BUILD, VGA_COL_SUCCESS);
}