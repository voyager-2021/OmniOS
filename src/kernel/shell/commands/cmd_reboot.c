#include "cmd_helpers.h"

void cmd_reboot(ShellCmd *cl)
{
    (void)cl;
    cprintln("  Rebooting OmniOS...", VGA_COL_WARNING);
    uint8_t tmp;
    do { tmp = i686_inb(0x64); } while (tmp & 0x02);
    i686_outb(0x64, 0xFE);
    __asm__ volatile("cli; hlt");
}