#include "cmd_helpers.h"

void cmd_meminfo(ShellCmd *cl)
{
    (void)cl;
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("   Memory Layout", VGA_COL_HEADER);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("  0x00000-0x003FF  IVT (Interrupt Vector Table)", VGA_COL_NORMAL);
    cprintln("  0x00400-0x004FF  BDA (BIOS Data Area)",         VGA_COL_NORMAL);
    cprintln("  0x07C00-0x07DFF  Stage1 MBR (512 bytes)",       VGA_COL_NORMAL);
    cprintln("  0x20000-0x2FFFF  Stage2 bootloader",            VGA_COL_NORMAL);
    cprintln("  0xB8000-0xBFFFF  VGA text framebuffer",         VGA_COL_NORMAL);
    cprintln("  0x100000+        Kernel (loaded above 1MB)",    VGA_COL_NORMAL);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
}