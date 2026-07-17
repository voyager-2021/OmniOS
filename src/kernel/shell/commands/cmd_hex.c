#include "cmd_helpers.h"

void cmd_hex(ShellCmd *cl)
{
    if (cl->argc < 2) { cprintln("  Usage: hex <addr> [len]", VGA_COL_WARNING); return; }
    uintptr_t addr = parse_addr(cl->argv[1]);
    int len = (cl->argc >= 3) ? cmd_atoi(cl->argv[2]) : 128;
    if (len <= 0 || len > 1024) len = 128;
    const uint8_t *mem = (const uint8_t *)addr;
    COLOR_SET(VGA_COL_HEADER);
    printf("  Hex @ 0x%08x (%d bytes)\n", (unsigned)addr, len);
    COLOR_RESET();
    hr('-', VGA_WIDTH, VGA_COL_NORMAL);
    for (int row = 0; row < len; row += 16) {
        COLOR_SET(VGA_COL_INFO); printf("  %08x  ", (unsigned)(addr + row)); COLOR_RESET();
        for (int b = 0; b < 16; b++) {
            if (row + b < len) printf("%02x ", mem[row + b]); else puts("   ");
            if (b == 7) putc(' ');
        }
        puts(" |");
        COLOR_SET(VGA_COL_SUCCESS);
        for (int b = 0; b < 16 && row + b < len; b++) {
            uint8_t byte = mem[row + b];
            putc((byte >= 0x20 && byte < 0x7F) ? (char)byte : '.');
        }
        COLOR_RESET();
        puts("|\n");
    }
}