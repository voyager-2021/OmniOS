#include "cmd_helpers.h"

void cmd_panic(ShellCmd *cl)
{
    (void)cl;
    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_WHITE, VGA_COLOR_RED));
    VGA_ClearScreen();
    puts("\n\n  *** KERNEL PANIC ***\n\n  Manual panic triggered.\n  System halted.\n\n  Reboot your machine.\n");
    __asm__ volatile("cli");
    while (1) __asm__ volatile("hlt");
}