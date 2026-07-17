#include "cmd_helpers.h"

void cmd_sysinfo(ShellCmd *cl)
{
    (void)cl;
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("   OmniOS System Information", VGA_COL_HEADER);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    kv_row("OS Name",      "OmniOS",                   16);
    kv_row("Version",      OMNIOS_VERSION,              16);
    kv_row("Build",        OMNIOS_BUILD,                16);
    kv_row("Architecture", "x86 (i686) 32-bit",         16);
    kv_row("CPU Mode",     "Protected Mode",             16);
    kv_row("Bootloader",   "Stage1 (MBR) + Stage2 ELF", 16);
    kv_row("Filesystem",   "FAT12/FAT16/FAT32",          16);
    kv_row("Display",      "VGA Text 80x25 @ 0xB8000",  16);
    kv_row("Keyboard",     "PS/2 IRQ1 ring buffer",      16);
    kv_row("Build System", "SCons + i686-elf-gcc",       16);
    int installed = 0;
    for (int i = 0; i < g_package_count; i++)
        if (g_packages[i].installed) installed++;
    COLOR_SET(VGA_COL_INFO);
    printf("  Packages        : %d/%d installed\n", installed, g_package_count);
    COLOR_RESET();
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
}