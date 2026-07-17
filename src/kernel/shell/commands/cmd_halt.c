#include "cmd_helpers.h"

void cmd_halt(ShellCmd *cl)
{
    (void)cl;
    cprintln("  OmniOS halted. Safe to power off.", VGA_COL_WARNING);
    __asm__ volatile("cli");
    while (1) __asm__ volatile("hlt");
}