/* ============================================================
   OmniOS - Command Implementations
   Commands: help, clear, cls, echo, version, sysinfo,
             meminfo, cpuinfo, history, color, calc, ascii,
             uptime, banner, hex, peek, poke, reboot, halt,
             panic
   ============================================================ */
#include "commands.h"
#include "shell.h"
#include "../stdio.h"
#include "../arch/i686/vga_text.h"
#include "../arch/i686/io.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define OMNIOS_VERSION  "0.1.0"
#define OMNIOS_BUILD    "2024.001"

/* ============================================================
   Utility macros / helpers
   ============================================================ */

#define COLOR_SET(c)   VGA_SetColor(c)
#define COLOR_RESET()  VGA_SetColor(VGA_COL_NORMAL)

static void cprint(const char *s, uint8_t col)
{
    COLOR_SET(col);
    puts(s);
    COLOR_RESET();
}

static void cprintln(const char *s, uint8_t col)
{
    COLOR_SET(col);
    puts(s);
    puts("\n");
    COLOR_RESET();
}

static void hr(char ch, int w, uint8_t col)
{
    COLOR_SET(col);
    for (int i = 0; i < w; i++) putc(ch);
    putc('\n');
    COLOR_RESET();
}

static int cmd_strlen(const char *s)
{
    int n = 0; while (s[n]) n++; return n;
}

static int cmd_strcmp(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

static int cmd_atoi(const char *s)
{
    int r = 0, neg = 0;
    if (*s == '-') { neg = 1; s++; }
    while (*s >= '0' && *s <= '9') r = r * 10 + (*s++ - '0');
    return neg ? -r : r;
}

/* Padded key-value row */
static void kv_row(const char *key, const char *val, int key_w)
{
    COLOR_SET(VGA_COL_INFO);
    puts("  ");
    puts(key);
    int pad = key_w - cmd_strlen(key);
    for (int i = 0; i < pad; i++) putc(' ');
    COLOR_RESET();
    puts(": ");
    puts(val);
    putc('\n');
}

/* ============================================================
   help
   ============================================================ */
void Cmd_Help(ShellCmd *cl)
{
    (void)cl;

    typedef struct { const char *name; const char *desc; } Entry;
    static const Entry cmds[] = {
        { "help",              "Show this help message"                      },
        { "clear / cls",       "Clear the terminal screen"                   },
        { "echo <text>",       "Print text to terminal"                      },
        { "version",           "Show OmniOS version string"                  },
        { "sysinfo",           "Display full system information"             },
        { "meminfo",           "Show memory layout information"              },
        { "cpuinfo",           "Show CPU / arch information"                 },
        { "history",           "List command history"                        },
        { "color <0-7>",       "Change terminal colour scheme"               },
        { "calc <expr>",       "Integer calculator  (e.g.  calc 10*3)"      },
        { "ascii",             "Print ASCII character table"                 },
        { "hex <addr> [len]",  "Hex-dump memory  (e.g.  hex 0xb8000 64)"   },
        { "peek <addr>",       "Read one byte from a physical address"       },
        { "poke <addr> <val>", "Write one byte to a physical address"        },
        { "uptime",            "Show system uptime (tick counter)"           },
        { "banner",            "Redraw the OmniOS boot banner"               },
        { "reboot",            "Reboot the system via keyboard controller"   },
        { "halt",              "Halt the CPU (safe power-off)"               },
        { "panic",             "Force a kernel panic (test)"                 },
        { NULL, NULL }
    };

    hr('=', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("   OmniOS Command Reference", VGA_COL_HEADER);
    hr('=', VGA_WIDTH, VGA_COL_HEADER);

    for (int i = 0; cmds[i].name; i++) {
        COLOR_SET(VGA_COL_SUCCESS);
        puts("  ");
        puts(cmds[i].name);
        int pad = 26 - cmd_strlen(cmds[i].name);
        for (int j = 0; j < pad; j++) putc(' ');
        COLOR_RESET();
        puts(cmds[i].desc);
        putc('\n');
    }

    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("  Arrow keys: move cursor / browse history  |  Home/End: jump", VGA_COL_INFO);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
}

/* ============================================================
   clear / cls
   ============================================================ */
void Cmd_Clear(ShellCmd *cl) { (void)cl; VGA_ClearScreen(); }
void Cmd_Cls(ShellCmd *cl)   { (void)cl; VGA_ClearScreen(); }

/* ============================================================
   echo
   ============================================================ */
void Cmd_Echo(ShellCmd *cl)
{
    for (int i = 1; i < cl->argc; i++) {
        puts(cl->argv[i]);
        if (i < cl->argc - 1) putc(' ');
    }
    putc('\n');
}

/* ============================================================
   version
   ============================================================ */
void Cmd_Version(ShellCmd *cl)
{
    (void)cl;
    cprint  ("  OmniOS v",       VGA_COL_INFO);
    cprint  (OMNIOS_VERSION,     VGA_COL_SUCCESS);
    cprint  ("  build ",         VGA_COL_INFO);
    cprintln(OMNIOS_BUILD,       VGA_COL_SUCCESS);
    cprintln("  Base: nanobyte_os (github.com/nanobyte-dev/nanobyte_os)", VGA_COL_NORMAL);
}

/* ============================================================
   sysinfo
   ============================================================ */
void Cmd_SysInfo(ShellCmd *cl)
{
    (void)cl;
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("   OmniOS System Information", VGA_COL_HEADER);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);

    kv_row("OS Name",       "OmniOS",                    16);
    kv_row("Version",       OMNIOS_VERSION,               16);
    kv_row("Build",         OMNIOS_BUILD,                 16);
    kv_row("Architecture",  "x86 (i686) 32-bit",          16);
    kv_row("CPU Mode",      "Protected Mode (PM)",         16);
    kv_row("Bootloader",    "Stage1 (MBR) + Stage2 ELF",  16);
    kv_row("Filesystem",    "FAT12/FAT16",                 16);
    kv_row("Display",       "VGA Text 80x25 @ 0xB8000",   16);
    kv_row("Keyboard",      "PS/2 IRQ1 with ring buffer",  16);
    kv_row("Build System",  "SCons + i686-elf-gcc",        16);
    kv_row("License",       "See LICENSE in repo",          16);

    hr('-', VGA_WIDTH, VGA_COL_HEADER);
}

/* ============================================================
   meminfo
   ============================================================ */
void Cmd_MemInfo(ShellCmd *cl)
{
    (void)cl;

    typedef struct { const char *region; const char *range; const char *note; } MemRow;
    static const MemRow rows[] = {
        { "Real Mode IVT",  "0x00000 - 0x003FF", "Interrupt Vector Table"          },
        { "BDA",            "0x00400 - 0x004FF", "BIOS Data Area"                  },
        { "Stage1 MBR",     "0x07C00 - 0x07DFF", "512-byte boot sector"            },
        { "Stage2 Loader",  "0x20000 - 0x2FFFF", "Second-stage bootloader"         },
        { "VGA Buffer",     "0xB8000 - 0xBFFFF", "Text mode framebuffer"           },
        { "Kernel",         "0x100000+",          "Loaded above 1MB by stage2"      },
        { "Heap (future)",  "Above kernel",        "kmalloc not yet implemented"    },
        { NULL, NULL, NULL }
    };

    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("   Memory Layout", VGA_COL_HEADER);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);

    COLOR_SET(VGA_COL_INFO);
    printf("  %-18s %-22s %s\n", "Region", "Address Range", "Notes");
    COLOR_RESET();
    hr('-', VGA_WIDTH, VGA_COL_NORMAL);

    for (int i = 0; rows[i].region; i++) {
        COLOR_SET(VGA_COL_SUCCESS);
        printf("  %-18s ", rows[i].range);
        COLOR_RESET();
        printf("%-22s %s\n", rows[i].region, rows[i].note);
    }
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("  Note: run 'sysinfo' after memory detection for real map", VGA_COL_WARNING);
}

/* ============================================================
   cpuinfo
   ============================================================ */
void Cmd_CpuInfo(ShellCmd *cl)
{
    (void)cl;
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("   CPU Information", VGA_COL_HEADER);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);

    kv_row("Architecture",  "x86 (IA-32 / i686)",         16);
    kv_row("Mode",          "32-bit Protected Mode",        16);
    kv_row("Segments",      "GDT: null/code/data (3 seg)", 16);
    kv_row("Interrupts",    "IDT: 256 entries loaded",      16);
    kv_row("PIC",           "i8259A  IRQ0-15 remapped",    16);
    kv_row("Keyboard IRQ",  "IRQ1 -> 0x21",                 16);
    kv_row("Timer IRQ",     "IRQ0 -> 0x20 (not hooked)",   16);
    kv_row("FPU",           "Not initialised",              16);
    kv_row("Paging",        "Disabled (flat model)",        16);
    kv_row("Cache",         "Unknown (no CPUID yet)",       16);

    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("  CPUID support planned for v0.2", VGA_COL_INFO);
}

/* ============================================================
   history  (shell history is kept in shell.c;
             commands.c receives a pointer via extern)
   ============================================================ */

/* Shell_Run lives in shell.c which manages the history array.
   We expose a small helper so commands.c can print it. */
extern const char *history_get(int offset);  /* defined in shell.c */
extern int         s_hist_count;             /* defined in shell.c */

void Cmd_History(ShellCmd *cl)
{
    (void)cl;
    cprintln("  Command History:", VGA_COL_HEADER);
    if (s_hist_count == 0) { cprintln("  (empty)", VGA_COL_WARNING); return; }

    for (int i = s_hist_count; i >= 1; i--) {
        const char *h = history_get(i);
        if (!h) continue;
        COLOR_SET(VGA_COL_INFO);
        printf("  %3d  ", s_hist_count - i + 1);
        COLOR_RESET();
        puts(h);
        putc('\n');
    }
}

/* ============================================================
   color
   ============================================================ */
void Cmd_Color(ShellCmd *cl)
{
    typedef struct { const char *name; uint8_t col; } Scheme;
    static const Scheme schemes[] = {
        { "Classic (grey/black)",   VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREY,    VGA_COLOR_BLACK) },
        { "Matrix  (green/black)",  VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN,   VGA_COLOR_BLACK) },
        { "Ocean   (cyan/black)",   VGA_MAKE_COLOR(VGA_COLOR_LIGHT_CYAN,    VGA_COLOR_BLACK) },
        { "Sunset  (red/black)",    VGA_MAKE_COLOR(VGA_COLOR_LIGHT_RED,     VGA_COLOR_BLACK) },
        { "Royal   (blue/black)",   VGA_MAKE_COLOR(VGA_COLOR_LIGHT_BLUE,    VGA_COLOR_BLACK) },
        { "Gold    (yellow/black)", VGA_MAKE_COLOR(VGA_COLOR_YELLOW,        VGA_COLOR_BLACK) },
        { "Mint    (white/blue)",   VGA_MAKE_COLOR(VGA_COLOR_WHITE,         VGA_COLOR_BLUE)  },
        { "Alert   (black/red)",    VGA_MAKE_COLOR(VGA_COLOR_WHITE,         VGA_COLOR_RED)   },
    };
    int num_schemes = (int)(sizeof(schemes) / sizeof(schemes[0]));

    if (cl->argc < 2) {
        cprintln("  Available colour schemes:", VGA_COL_HEADER);
        for (int i = 0; i < num_schemes; i++) {
            COLOR_SET(schemes[i].col);
            printf("  %d  %s\n", i, schemes[i].name);
        }
        COLOR_RESET();
        cprintln("\n  Usage: color <0-7>", VGA_COL_WARNING);
        return;
    }

    int n = cmd_atoi(cl->argv[1]);
    if (n < 0 || n >= num_schemes) {
        cprintln("  Invalid scheme number. Run 'color' to list.", VGA_COL_ERROR);
        return;
    }

    VGA_SetColor(schemes[n].col);
    VGA_ClearScreen();
    printf("  Colour scheme %d (%s) applied.\n", n, schemes[n].name);
}

/* ============================================================
   calc  — supports + - * / % with integers
   ============================================================ */
void Cmd_Calc(ShellCmd *cl)
{
    if (cl->argc < 2) {
        cprintln("  Usage: calc <expression>  e.g.  calc 10+3  or  calc 10 * 3", VGA_COL_WARNING);
        return;
    }

    /* Rebuild expression from args (allow spaces between tokens) */
    static char expr[128];
    int ep = 0;
    for (int i = 1; i < cl->argc && ep < 126; i++)
        for (int j = 0; cl->argv[i][j] && ep < 126; j++)
            expr[ep++] = cl->argv[i][j];
    expr[ep] = '\0';

    /* Parse: optional sign, number, operator, optional sign, number */
    char *p = expr;
    bool neg_a = false;
    if (*p == '-') { neg_a = true; p++; }
    else if (*p == '+') { p++; }

    int64_t a = 0;
    while (*p >= '0' && *p <= '9') a = a * 10 + (*p++ - '0');
    if (neg_a) a = -a;

    char op = *p++;
    if (!op) {
        /* single number, just print it */
        COLOR_SET(VGA_COL_SUCCESS);
        printf("  = %d\n", (int)a);
        COLOR_RESET();
        return;
    }

    bool neg_b = false;
    if (*p == '-') { neg_b = true; p++; }
    else if (*p == '+') { p++; }

    int64_t b = 0;
    while (*p >= '0' && *p <= '9') b = b * 10 + (*p++ - '0');
    if (neg_b) b = -b;

    int64_t result = 0;
    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': case 'x': result = a * b; break;
        case '/':
            if (b == 0) { cprintln("  Error: division by zero", VGA_COL_ERROR); return; }
            result = a / b; break;
        case '%':
            if (b == 0) { cprintln("  Error: modulo by zero",   VGA_COL_ERROR); return; }
            result = a % b; break;
        default:
            COLOR_SET(VGA_COL_ERROR);
            printf("  Unknown operator '%c'. Use + - * / %%\n", op);
            COLOR_RESET();
            return;
    }

    COLOR_SET(VGA_COL_SUCCESS);
    printf("  %d %c %d = %d\n", (int)a, op, (int)b, (int)result);
    COLOR_RESET();
}

/* ============================================================
   ascii
   ============================================================ */
void Cmd_Ascii(ShellCmd *cl)
{
    (void)cl;
    cprintln("  ASCII Table (32 - 126):", VGA_COL_HEADER);
    hr('-', VGA_WIDTH, VGA_COL_NORMAL);

    int col = 0;
    for (int i = 32; i <= 126; i++) {
        COLOR_SET(VGA_COL_INFO);
        printf(" %3d", i);
        COLOR_RESET();
        printf(" %c  ", (char)i);
        col++;
        if (col % 10 == 0) putc('\n');
    }
    if (col % 10 != 0) putc('\n');
    hr('-', VGA_WIDTH, VGA_COL_NORMAL);
}

/* ============================================================
   hex  —  hex dump of physical memory
   ============================================================ */
void Cmd_Hex(ShellCmd *cl)
{
    if (cl->argc < 2) {
        cprintln("  Usage: hex <address> [length]", VGA_COL_WARNING);
        cprintln("  Example: hex 0xb8000 128", VGA_COL_INFO);
        return;
    }

    /* Parse address (hex with optional 0x prefix, or decimal) */
    const char *astr = cl->argv[1];
    uintptr_t addr = 0;
    if (astr[0] == '0' && (astr[1] == 'x' || astr[1] == 'X')) {
        astr += 2;
        while (*astr) {
            char c = *astr++;
            if (c >= '0' && c <= '9') addr = addr * 16 + (c - '0');
            else if (c >= 'a' && c <= 'f') addr = addr * 16 + (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') addr = addr * 16 + (c - 'A' + 10);
        }
    } else {
        addr = (uintptr_t)cmd_atoi(astr);
    }

    int len = 128;
    if (cl->argc >= 3) len = cmd_atoi(cl->argv[2]);
    if (len <= 0 || len > 1024) len = 128;

    const uint8_t *mem = (const uint8_t *)addr;

    COLOR_SET(VGA_COL_HEADER);
    printf("  Hex dump @ 0x%08x  (%d bytes)\n", (unsigned)addr, len);
    COLOR_RESET();
    hr('-', VGA_WIDTH, VGA_COL_NORMAL);

    for (int row = 0; row < len; row += 16) {
        COLOR_SET(VGA_COL_INFO);
        printf("  %08x  ", (unsigned)(addr + row));
        COLOR_RESET();

        /* Hex bytes */
        for (int b = 0; b < 16; b++) {
            if (row + b < len) {
                printf("%02x ", mem[row + b]);
            } else {
                puts("   ");
            }
            if (b == 7) putc(' ');
        }

        puts(" |");
        /* ASCII */
        COLOR_SET(VGA_COL_SUCCESS);
        for (int b = 0; b < 16 && row + b < len; b++) {
            uint8_t byte = mem[row + b];
            putc((byte >= 0x20 && byte < 0x7F) ? (char)byte : '.');
        }
        COLOR_RESET();
        puts("|\n");
    }
    hr('-', VGA_WIDTH, VGA_COL_NORMAL);
}

/* ============================================================
   peek  —  read one byte
   ============================================================ */
void Cmd_Peek(ShellCmd *cl)
{
    if (cl->argc < 2) {
        cprintln("  Usage: peek <address>  e.g. peek 0xb8000", VGA_COL_WARNING);
        return;
    }
    const char *astr = cl->argv[1];
    uintptr_t addr = 0;
    if (astr[0] == '0' && (astr[1] == 'x' || astr[1] == 'X')) {
        astr += 2;
        while (*astr) {
            char c = *astr++;
            if (c >= '0' && c <= '9') addr = addr * 16 + (c - '0');
            else if (c >= 'a' && c <= 'f') addr = addr * 16 + (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') addr = addr * 16 + (c - 'A' + 10);
        }
    } else {
        addr = (uintptr_t)cmd_atoi(cl->argv[1]);
    }
    uint8_t val = *(volatile uint8_t *)addr;
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  [0x%08x] = 0x%02x  (%d)  '%c'\n",
           (unsigned)addr, val, val,
           (val >= 0x20 && val < 0x7F) ? (char)val : '.');
    COLOR_RESET();
}

/* ============================================================
   poke  —  write one byte
   ============================================================ */
void Cmd_Poke(ShellCmd *cl)
{
    if (cl->argc < 3) {
        cprintln("  Usage: poke <address> <value>  e.g. poke 0xb8000 65", VGA_COL_WARNING);
        return;
    }
    const char *astr = cl->argv[1];
    uintptr_t addr = 0;
    if (astr[0] == '0' && (astr[1] == 'x' || astr[1] == 'X')) {
        astr += 2;
        while (*astr) {
            char c = *astr++;
            if (c >= '0' && c <= '9') addr = addr * 16 + (c - '0');
            else if (c >= 'a' && c <= 'f') addr = addr * 16 + (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') addr = addr * 16 + (c - 'A' + 10);
        }
    } else {
        addr = (uintptr_t)cmd_atoi(cl->argv[1]);
    }
    uint8_t val = (uint8_t)cmd_atoi(cl->argv[2]);
    *(volatile uint8_t *)addr = val;
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  Wrote 0x%02x to [0x%08x]\n", val, (unsigned)addr);
    COLOR_RESET();
}

/* ============================================================
   uptime  (tick counter via IRQ0 when available)
   ============================================================ */
extern volatile uint32_t g_tick_count;   /* defined in kernel main or timer driver */

void Cmd_Uptime(ShellCmd *cl)
{
    (void)cl;
    COLOR_SET(VGA_COL_INFO);
    printf("  System ticks: %u\n", g_tick_count);
    uint32_t secs = g_tick_count / 18; /* ~18.2 Hz PIT default */
    printf("  Approx time : %u seconds since boot\n", secs);
    COLOR_RESET();
}

/* ============================================================
   banner
   ============================================================ */
void Cmd_Banner(ShellCmd *cl)
{
    (void)cl;
    VGA_ClearScreen();
    hr('*', VGA_WIDTH, VGA_COL_HEADER);

    COLOR_SET(VGA_COL_HEADER);
    puts("\n");
    puts("   ..|'''.|                   .|'''.|  \n");
    puts("  .|'     '    mm    nnnn     ||..  '  \n");
    puts("  ||    ....  'MM'   nn ''Nn  ''|||.   \n");
    puts("  '|.    ||  .M''M.  nn   MM  .   '||  \n");
    puts("   ''|...'| .M'  'Mb.MM  .M'  |'...|'  \n");
    puts("\n");
    COLOR_RESET();

    COLOR_SET(VGA_COL_INFO);
    printf("   Terminal Operating System  v%s  |  Build %s\n",
           OMNIOS_VERSION, OMNIOS_BUILD);
    COLOR_RESET();

    puts("\n");
    hr('*', VGA_WIDTH, VGA_COL_HEADER);

    cprintln("  Type 'help' for a command list.", VGA_COL_INFO);
    hr('-', VGA_WIDTH, VGA_COL_INFO);
}

/* ============================================================
   reboot
   ============================================================ */
void Cmd_Reboot(ShellCmd *cl)
{
    (void)cl;
    cprintln("  Rebooting OmniOS...", VGA_COL_WARNING);
    /* Drain keyboard controller input buffer then pulse reset */
    uint8_t tmp;
    do { tmp = i686_inb(0x64); } while (tmp & 0x02);
    i686_outb(0x64, 0xFE);
    /* If that didn't work, triple fault */
    __asm__ volatile("cli; hlt");
}

/* ============================================================
   halt
   ============================================================ */
void Cmd_Halt(ShellCmd *cl)
{
    (void)cl;
    cprintln("  OmniOS halted. Safe to power off.", VGA_COL_WARNING);
    __asm__ volatile("cli");
    while (1) __asm__ volatile("hlt");
}

/* ============================================================
   panic
   ============================================================ */
void Cmd_Panic(ShellCmd *cl)
{
    (void)cl;
    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_WHITE, VGA_COLOR_RED));
    VGA_ClearScreen();
    puts("\n\n");
    puts("  *** KERNEL PANIC ***\n\n");
    puts("  Manual panic triggered from shell.\n");
    puts("  OmniOS has halted.\n\n");
    puts("  Reboot your machine.\n");
    __asm__ volatile("cli");
    while (1) __asm__ volatile("hlt");
}

/* ============================================================
   Dispatch table
   ============================================================ */
typedef struct {
    const char *name;
    void (*fn)(ShellCmd *);
} CmdEntry;

static const CmdEntry s_dispatch[] = {
    { "help",    Cmd_Help    },
    { "?",       Cmd_Help    },
    { "clear",   Cmd_Clear   },
    { "cls",     Cmd_Cls     },
    { "echo",    Cmd_Echo    },
    { "version", Cmd_Version },
    { "ver",     Cmd_Version },
    { "sysinfo", Cmd_SysInfo },
    { "meminfo", Cmd_MemInfo },
    { "cpuinfo", Cmd_CpuInfo },
    { "history", Cmd_History },
    { "hist",    Cmd_History },
    { "color",   Cmd_Color   },
    { "colour",  Cmd_Color   },
    { "calc",    Cmd_Calc    },
    { "ascii",   Cmd_Ascii   },
    { "hex",     Cmd_Hex     },
    { "peek",    Cmd_Peek    },
    { "poke",    Cmd_Poke    },
    { "uptime",  Cmd_Uptime  },
    { "banner",  Cmd_Banner  },
    { "reboot",  Cmd_Reboot  },
    { "halt",    Cmd_Halt    },
    { "panic",   Cmd_Panic   },
    { NULL,      NULL        }
};

void Commands_Dispatch(ShellCmd *cl)
{
    if (!cl || !cl->cmd[0]) return;

    for (int i = 0; s_dispatch[i].name; i++) {
        if (cmd_strcmp(cl->cmd, s_dispatch[i].name) == 0) {
            s_dispatch[i].fn(cl);
            return;
        }
    }

    /* Unknown command */
    COLOR_SET(VGA_COL_ERROR);
    printf("  Unknown command: '%s'", cl->cmd);
    COLOR_RESET();
    cprintln("  Type 'help' to list available commands.", VGA_COL_INFO);
}