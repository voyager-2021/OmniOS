#include "cmd_helpers.h"

void cmd_peek(ShellCmd *cl)
{
    if (cl->argc < 2) { cprintln("  Usage: peek <addr>", VGA_COL_WARNING); return; }
    uintptr_t addr = parse_addr(cl->argv[1]);
    uint8_t val = *(volatile uint8_t *)addr;
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  [0x%08x] = 0x%02x (%d) '%c'\n", (unsigned)addr, val, val,
           (val >= 0x20 && val < 0x7F) ? (char)val : '.');
    COLOR_RESET();
}