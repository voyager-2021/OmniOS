#include "cmd_helpers.h"

void cmd_cpuinfo(ShellCmd *cl)
{
    (void)cl;
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("   CPU Information", VGA_COL_HEADER);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    kv_row("Architecture", "x86 (IA-32 / i686)",       16);
    kv_row("Mode",         "32-bit Protected Mode",      16);
    kv_row("GDT",          "3 segments loaded",          16);
    kv_row("IDT",          "256 entries",                16);
    kv_row("PIC",          "i8259A remapped 0x20-0x2F", 16);
    kv_row("Paging",       "Disabled (flat model)",      16);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
}