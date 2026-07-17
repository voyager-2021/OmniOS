#include "cmd_helpers.h"

void cmd_poke(ShellCmd *cl)
{
    if (cl->argc < 3) { cprintln("  Usage: poke <addr> <val>", VGA_COL_WARNING); return; }
    uintptr_t addr = parse_addr(cl->argv[1]);
    uint8_t val = (uint8_t)cmd_atoi(cl->argv[2]);
    *(volatile uint8_t *)addr = val;
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  Wrote 0x%02x to [0x%08x]\n", val, (unsigned)addr);
    COLOR_RESET();
}