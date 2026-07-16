/* ============================================================
   OmniOS v1.0.0 - Command Implementations
   ============================================================ */
#include "commands.h"
#include "shell.h"
#include "../stdio.h"
#include "../arch/i686/vga_text.h"
#include "../arch/i686/io.h"
#include "../drivers/keyboard.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define COLOR_SET(c)   VGA_SetColor(c)
#define COLOR_RESET()  VGA_SetColor(VGA_COL_NORMAL)

/* ---- Helpers ---- */
static void cprint(const char *s, uint8_t col)
{
    COLOR_SET(col); puts(s); COLOR_RESET();
}

static void cprintln(const char *s, uint8_t col)
{
    COLOR_SET(col); puts(s); puts("\n"); COLOR_RESET();
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

static void kv_row(const char *key, const char *val, int key_w)
{
    COLOR_SET(VGA_COL_INFO);
    puts("  "); puts(key);
    int pad = key_w - cmd_strlen(key);
    for (int i = 0; i < pad; i++) putc(' ');
    COLOR_RESET();
    puts(": "); puts(val); putc('\n');
}

static uintptr_t parse_addr(const char *s)
{
    uintptr_t addr = 0;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
        while (*s) {
            char c = *s++;
            if      (c >= '0' && c <= '9') addr = addr * 16 + (c - '0');
            else if (c >= 'a' && c <= 'f') addr = addr * 16 + (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') addr = addr * 16 + (c - 'A' + 10);
            else break;
        }
    } else {
        addr = (uintptr_t)cmd_atoi(s);
    }
    return addr;
}

/* ============================================================
   Commands
   ============================================================ */

static void cmd_help(ShellCmd *cl)
{
    (void)cl;
    typedef struct { const char *name; const char *desc; } E;
    static const E cmds[] = {
        { "help",              "Show this help message"                   },
        { "clear / cls",       "Clear the terminal screen"                },
        { "echo <text>",       "Print text to terminal"                   },
        { "version",           "Show OmniOS version"                      },
        { "sysinfo",           "Display system information"               },
        { "meminfo",           "Show memory layout"                       },
        { "cpuinfo",           "Show CPU / arch information"              },
        { "history",           "List command history"                     },
        { "color <0-7>",       "Change colour scheme"                     },
        { "calc <expr>",       "Integer calculator (e.g. calc 10+3)"     },
        { "ascii",             "Print ASCII table"                        },
        { "hex <addr> [len]",  "Hex dump memory"                          },
        { "peek <addr>",       "Read byte from address"                   },
        { "poke <addr> <val>", "Write byte to address"                    },
        { "uptime",            "Show tick counter"                        },
        { "date",              "Show system date/time (from ticks)"       },
        { "whoami",            "Show current user"                        },
        { "hostname",          "Show/set hostname"                        },
        { "banner",            "Redraw boot banner"                       },
        { "packages",          "List available packages"                  },
        { "install <pkg>",     "Install a package"                        },
        { "uninstall <pkg>",   "Uninstall a package"                      },
        { "run <pkg> [args]",  "Run an installed package"                 },
        { "reboot",            "Reboot the system"                        },
        { "halt / shutdown",   "Halt the CPU"                             },
        { "panic",             "Trigger kernel panic (test)"              },
        { NULL, NULL }
    };

    hr('=', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("   OmniOS v" OMNIOS_VERSION " Command Reference", VGA_COL_HEADER);
    hr('=', VGA_WIDTH, VGA_COL_HEADER);

    for (int i = 0; cmds[i].name; i++) {
        COLOR_SET(VGA_COL_SUCCESS);
        puts("  "); puts(cmds[i].name);
        int pad = 26 - cmd_strlen(cmds[i].name);
        for (int j = 0; j < pad; j++) putc(' ');
        COLOR_RESET();
        puts(cmds[i].desc); putc('\n');
    }

    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("  Installed packages also work as commands.", VGA_COL_INFO);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
}

static void cmd_clear(ShellCmd *cl) { (void)cl; VGA_ClearScreen(); }

static void cmd_echo(ShellCmd *cl)
{
    for (int i = 1; i < cl->argc; i++) {
        puts(cl->argv[i]);
        if (i < cl->argc - 1) putc(' ');
    }
    putc('\n');
}

static void cmd_version(ShellCmd *cl)
{
    (void)cl;
    cprint("  OmniOS v", VGA_COL_INFO);
    cprint(OMNIOS_VERSION, VGA_COL_SUCCESS);
    cprint("  build ", VGA_COL_INFO);
    cprintln(OMNIOS_BUILD, VGA_COL_SUCCESS);
}

static void cmd_sysinfo(ShellCmd *cl)
{
    (void)cl;
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("   OmniOS System Information", VGA_COL_HEADER);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    kv_row("OS Name",       "OmniOS",                    16);
    kv_row("Version",       OMNIOS_VERSION,               16);
    kv_row("Build",         OMNIOS_BUILD,                 16);
    kv_row("Architecture",  "x86 (i686) 32-bit",          16);
    kv_row("CPU Mode",      "Protected Mode",              16);
    kv_row("Bootloader",    "Stage1 (MBR) + Stage2 ELF",  16);
    kv_row("Filesystem",    "FAT12/FAT16/FAT32",           16);
    kv_row("Display",       "VGA Text 80x25 @ 0xB8000",   16);
    kv_row("Keyboard",      "PS/2 IRQ1 ring buffer",       16);
    kv_row("Build System",  "SCons + i686-elf-gcc",        16);

    /* Show installed packages count */
    int installed = 0;
    for (int i = 0; i < g_package_count; i++)
        if (g_packages[i].installed) installed++;
    COLOR_SET(VGA_COL_INFO);
    printf("  Packages        : %d/%d installed\n", installed, g_package_count);
    COLOR_RESET();

    hr('-', VGA_WIDTH, VGA_COL_HEADER);
}

static void cmd_meminfo(ShellCmd *cl)
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

static void cmd_cpuinfo(ShellCmd *cl)
{
    (void)cl;
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("   CPU Information", VGA_COL_HEADER);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    kv_row("Architecture",  "x86 (IA-32 / i686)",         16);
    kv_row("Mode",          "32-bit Protected Mode",        16);
    kv_row("GDT",           "3 segments loaded",            16);
    kv_row("IDT",           "256 entries",                  16);
    kv_row("PIC",           "i8259A remapped 0x20-0x2F",   16);
    kv_row("Paging",        "Disabled (flat model)",        16);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
}

static void cmd_history(ShellCmd *cl)
{
    (void)cl;
    cprintln("  Command History:", VGA_COL_HEADER);
    if (s_hist_count == 0) {
        cprintln("  (empty)", VGA_COL_WARNING);
        return;
    }
    for (int i = s_hist_count; i >= 1; i--) {
        const char *h = history_get(i);
        if (!h) continue;
        COLOR_SET(VGA_COL_INFO);
        printf("  %3d  ", s_hist_count - i + 1);
        COLOR_RESET();
        puts(h); putc('\n');
    }
}

static void cmd_color(ShellCmd *cl)
{
    typedef struct { const char *name; uint8_t col; } S;
    static const S schemes[] = {
        { "Classic (grey/black)",   VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREY,  VGA_COLOR_BLACK) },
        { "Matrix  (green/black)",  VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK) },
        { "Ocean   (cyan/black)",   VGA_MAKE_COLOR(VGA_COLOR_LIGHT_CYAN,  VGA_COLOR_BLACK) },
        { "Sunset  (red/black)",    VGA_MAKE_COLOR(VGA_COLOR_LIGHT_RED,   VGA_COLOR_BLACK) },
        { "Royal   (blue/black)",   VGA_MAKE_COLOR(VGA_COLOR_LIGHT_BLUE,  VGA_COLOR_BLACK) },
        { "Gold    (yellow/black)", VGA_MAKE_COLOR(VGA_COLOR_YELLOW,      VGA_COLOR_BLACK) },
        { "Mint    (white/blue)",   VGA_MAKE_COLOR(VGA_COLOR_WHITE,       VGA_COLOR_BLUE)  },
        { "Alert   (white/red)",    VGA_MAKE_COLOR(VGA_COLOR_WHITE,       VGA_COLOR_RED)   },
    };
    int num = (int)(sizeof(schemes) / sizeof(schemes[0]));

    if (cl->argc < 2) {
        cprintln("  Colour schemes:", VGA_COL_HEADER);
        for (int i = 0; i < num; i++) {
            COLOR_SET(schemes[i].col);
            printf("  %d  %s\n", i, schemes[i].name);
        }
        COLOR_RESET();
        cprintln("\n  Usage: color <0-7>", VGA_COL_WARNING);
        return;
    }
    int n = cmd_atoi(cl->argv[1]);
    if (n < 0 || n >= num) {
        cprintln("  Invalid scheme. Run 'color' to list.", VGA_COL_ERROR);
        return;
    }
    VGA_SetColor(schemes[n].col);
    VGA_ClearScreen();
    printf("  Colour scheme %d applied.\n", n);
}

static void cmd_calc(ShellCmd *cl)
{
    if (cl->argc < 2) {
        cprintln("  Usage: calc <expr>  e.g. calc 10+3", VGA_COL_WARNING);
        return;
    }
    static char expr[128];
    int ep = 0;
    for (int i = 1; i < cl->argc && ep < 126; i++)
        for (int j = 0; cl->argv[i][j] && ep < 126; j++)
            expr[ep++] = cl->argv[i][j];
    expr[ep] = '\0';

    char *p = expr;
    bool neg_a = 0;
    if (*p == '-') { neg_a = 1; p++; }
    else if (*p == '+') p++;

    int64_t a = 0;
    while (*p >= '0' && *p <= '9') a = a * 10 + (*p++ - '0');
    if (neg_a) a = -a;

    char op = *p++;
    if (!op) {
        COLOR_SET(VGA_COL_SUCCESS);
        printf("  = %d\n", (int)a);
        COLOR_RESET();
        return;
    }

    bool neg_b = 0;
    if (*p == '-') { neg_b = 1; p++; }
    else if (*p == '+') p++;

    int64_t b = 0;
    while (*p >= '0' && *p <= '9') b = b * 10 + (*p++ - '0');
    if (neg_b) b = -b;

    int64_t result = 0;
    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': case 'x': result = a * b; break;
        case '/':
            if (b == 0) { cprintln("  Error: div by zero", VGA_COL_ERROR); return; }
            result = a / b; break;
        case '%':
            if (b == 0) { cprintln("  Error: mod by zero", VGA_COL_ERROR); return; }
            result = a % b; break;
        default:
            COLOR_SET(VGA_COL_ERROR);
            printf("  Unknown operator '%c'\n", op);
            COLOR_RESET();
            return;
    }
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  %d %c %d = %d\n", (int)a, op, (int)b, (int)result);
    COLOR_RESET();
}

static void cmd_ascii(ShellCmd *cl)
{
    (void)cl;
    cprintln("  ASCII Table (32-126):", VGA_COL_HEADER);
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
}

static void cmd_hex(ShellCmd *cl)
{
    if (cl->argc < 2) {
        cprintln("  Usage: hex <addr> [len]  e.g. hex 0xb8000 64", VGA_COL_WARNING);
        return;
    }
    uintptr_t addr = parse_addr(cl->argv[1]);
    int len = (cl->argc >= 3) ? cmd_atoi(cl->argv[2]) : 128;
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
        for (int b = 0; b < 16; b++) {
            if (row + b < len) printf("%02x ", mem[row + b]);
            else puts("   ");
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

static void cmd_peek(ShellCmd *cl)
{
    if (cl->argc < 2) {
        cprintln("  Usage: peek <addr>", VGA_COL_WARNING);
        return;
    }
    uintptr_t addr = parse_addr(cl->argv[1]);
    uint8_t val = *(volatile uint8_t *)addr;
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  [0x%08x] = 0x%02x (%d) '%c'\n",
           (unsigned)addr, val, val,
           (val >= 0x20 && val < 0x7F) ? (char)val : '.');
    COLOR_RESET();
}

static void cmd_poke(ShellCmd *cl)
{
    if (cl->argc < 3) {
        cprintln("  Usage: poke <addr> <val>", VGA_COL_WARNING);
        return;
    }
    uintptr_t addr = parse_addr(cl->argv[1]);
    uint8_t val = (uint8_t)cmd_atoi(cl->argv[2]);
    *(volatile uint8_t *)addr = val;
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  Wrote 0x%02x to [0x%08x]\n", val, (unsigned)addr);
    COLOR_RESET();
}

extern volatile uint32_t g_tick_count;

static void cmd_uptime(ShellCmd *cl)
{
    (void)cl;
    uint32_t ticks = g_tick_count;
    uint32_t secs = ticks / 18;
    uint32_t mins = secs / 60;
    uint32_t hrs = mins / 60;
    COLOR_SET(VGA_COL_INFO);
    printf("  Uptime: %02u:%02u:%02u  (%u ticks)\n", hrs, mins % 60, secs % 60, ticks);
    COLOR_RESET();
}

static void cmd_date(ShellCmd *cl)
{
    (void)cl;
    uint32_t secs = g_tick_count / 18;
    uint32_t mins = secs / 60;
    uint32_t hrs = mins / 60;
    COLOR_SET(VGA_COL_INFO);
    printf("  System time: %02u:%02u:%02u (since boot)\n", hrs, mins % 60, secs % 60);
    COLOR_RESET();
    cprintln("  Note: No RTC driver. Time is relative to boot.", VGA_COL_WARNING);
}

static char g_hostname[32] = "omnios";

static void cmd_whoami(ShellCmd *cl)
{
    (void)cl;
    COLOR_SET(VGA_COL_SUCCESS);
    puts("  root@");
    puts(g_hostname);
    puts("\n");
    COLOR_RESET();
}

static void cmd_hostname(ShellCmd *cl)
{
    if (cl->argc >= 2) {
        /* Set hostname */
        int i = 0;
        const char *s = cl->argv[1];
        while (*s && i < 30) g_hostname[i++] = *s++;
        g_hostname[i] = '\0';
        COLOR_SET(VGA_COL_SUCCESS);
        printf("  Hostname set to: %s\n", g_hostname);
        COLOR_RESET();
    } else {
        COLOR_SET(VGA_COL_INFO);
        printf("  Hostname: %s\n", g_hostname);
        COLOR_RESET();
    }
}

static void cmd_banner(ShellCmd *cl)
{
    (void)cl;
    VGA_ClearScreen();
    hr('*', VGA_WIDTH, VGA_COL_HEADER);
    COLOR_SET(VGA_COL_HEADER);
    puts("\n");
    puts("     ::::::::  :::   ::: ::::    ::: :::::::::  ::::::::  :::::\n");
    puts("    :+:    :+: :+:  :+: :+:+:   :+:  :+:    :+::+:    :+::+:  \n");
    puts("    +#+    +:+ +#++:+++ +#+ +:+ +#+  +#+    +:++#+    +:++#++:\n");
    puts("    #+#    #+# #+#  #+# #+#   #+#+#  #+#    #+##+#    #+#    #+#\n");
    puts("     ########  ###  ### ###    ####  #########  ########  :::::\n");
    puts("\n");
    COLOR_RESET();
    COLOR_SET(VGA_COL_INFO);
    printf("   Terminal OS  v%s  |  Build %s\n", OMNIOS_VERSION, OMNIOS_BUILD);
    COLOR_RESET();
    puts("\n");
    hr('*', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("  Type 'help' for commands.", VGA_COL_INFO);
}

/* ---- Package commands ---- */

static void cmd_packages(ShellCmd *cl)
{
    (void)cl;
    hr('=', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("   OmniOS Package Manager", VGA_COL_HEADER);
    hr('=', VGA_WIDTH, VGA_COL_HEADER);

    if (g_package_count == 0) {
        cprintln("  No packages available.", VGA_COL_WARNING);
        return;
    }

    COLOR_SET(VGA_COL_INFO);
    printf("  %-16s %-8s %-10s %s\n", "PACKAGE", "VER", "STATUS", "DESCRIPTION");
    COLOR_RESET();
    hr('-', VGA_WIDTH, VGA_COL_NORMAL);

    for (int i = 0; i < g_package_count; i++) {
        Package *p = &g_packages[i];
        if (p->installed) {
            COLOR_SET(VGA_COL_SUCCESS);
            printf("  %-16s %-8s ", p->name, p->version);
            cprint("[installed]", VGA_COL_SUCCESS);
        } else {
            COLOR_SET(VGA_COL_NORMAL);
            printf("  %-16s %-8s ", p->name, p->version);
            cprint("[  ready  ]", VGA_COL_WARNING);
        }
        COLOR_SET(VGA_COL_NORMAL);
        printf(" %s\n", p->description);
    }

    hr('-', VGA_WIDTH, VGA_COL_NORMAL);
    cprintln("  Use: install <name>  |  uninstall <name>  |  <name> [args]", VGA_COL_INFO);
}

static void cmd_install(ShellCmd *cl)
{
    if (cl->argc < 2) {
        cprintln("  Usage: install <package>", VGA_COL_WARNING);
        cprintln("  Run 'packages' to see available packages.", VGA_COL_INFO);
        return;
    }

    const char *name = cl->argv[1];

    if (cmd_strcmp(name, "all") == 0) {
        /* Install all packages */
        int count = 0;
        for (int i = 0; i < g_package_count; i++) {
            if (!g_packages[i].installed) {
                g_packages[i].installed = true;
                COLOR_SET(VGA_COL_SUCCESS);
                printf("  Installed: %s v%s\n", g_packages[i].name, g_packages[i].version);
                COLOR_RESET();
                count++;
            }
        }
        if (count == 0) cprintln("  All packages already installed.", VGA_COL_INFO);
        else {
            COLOR_SET(VGA_COL_SUCCESS);
            printf("  %d package(s) installed.\n", count);
            COLOR_RESET();
        }
        return;
    }

    int idx = PKG_Find(name);
    if (idx < 0) {
        COLOR_SET(VGA_COL_ERROR);
        printf("  Package '%s' not found.\n", name);
        COLOR_RESET();
        cprintln("  Run 'packages' to see available packages.", VGA_COL_INFO);
        return;
    }

    if (g_packages[idx].installed) {
        COLOR_SET(VGA_COL_WARNING);
        printf("  Package '%s' is already installed.\n", name);
        COLOR_RESET();
        return;
    }

    PKG_Install(name);
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  Successfully installed: %s v%s\n", g_packages[idx].name, g_packages[idx].version);
    printf("  Run with: %s\n", g_packages[idx].name);
    COLOR_RESET();
}

static void cmd_uninstall(ShellCmd *cl)
{
    if (cl->argc < 2) {
        cprintln("  Usage: uninstall <package>", VGA_COL_WARNING);
        return;
    }

    const char *name = cl->argv[1];

    if (cmd_strcmp(name, "all") == 0) {
        int count = 0;
        for (int i = 0; i < g_package_count; i++) {
            if (g_packages[i].installed) {
                g_packages[i].installed = false;
                count++;
            }
        }
        COLOR_SET(VGA_COL_SUCCESS);
        printf("  %d package(s) uninstalled.\n", count);
        COLOR_RESET();
        return;
    }

    int idx = PKG_Find(name);
    if (idx < 0) {
        COLOR_SET(VGA_COL_ERROR);
        printf("  Package '%s' not found.\n", name);
        COLOR_RESET();
        return;
    }

    if (!g_packages[idx].installed) {
        COLOR_SET(VGA_COL_WARNING);
        printf("  Package '%s' is not installed.\n", name);
        COLOR_RESET();
        return;
    }

    PKG_Uninstall(name);
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  Uninstalled: %s\n", name);
    COLOR_RESET();
}

static void cmd_reboot(ShellCmd *cl)
{
    (void)cl;
    cprintln("  Rebooting OmniOS...", VGA_COL_WARNING);
    uint8_t tmp;
    do { tmp = i686_inb(0x64); } while (tmp & 0x02);
    i686_outb(0x64, 0xFE);
    __asm__ volatile("cli; hlt");
}

static void cmd_halt(ShellCmd *cl)
{
    (void)cl;
    cprintln("  OmniOS halted. Safe to power off.", VGA_COL_WARNING);
    __asm__ volatile("cli");
    while (1) __asm__ volatile("hlt");
}

static void cmd_panic(ShellCmd *cl)
{
    (void)cl;
    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_WHITE, VGA_COLOR_RED));
    VGA_ClearScreen();
    puts("\n\n");
    puts("  *** KERNEL PANIC ***\n\n");
    puts("  Manual panic triggered from shell.\n");
    puts("  System halted.\n\n");
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
    { "help",      cmd_help      },
    { "?",         cmd_help      },
    { "clear",     cmd_clear     },
    { "cls",       cmd_clear     },
    { "echo",      cmd_echo      },
    { "version",   cmd_version   },
    { "ver",       cmd_version   },
    { "sysinfo",   cmd_sysinfo   },
    { "meminfo",   cmd_meminfo   },
    { "cpuinfo",   cmd_cpuinfo   },
    { "history",   cmd_history   },
    { "hist",      cmd_history   },
    { "color",     cmd_color     },
    { "colour",    cmd_color     },
    { "calc",      cmd_calc      },
    { "ascii",     cmd_ascii     },
    { "hex",       cmd_hex       },
    { "peek",      cmd_peek      },
    { "poke",      cmd_poke      },
    { "uptime",    cmd_uptime    },
    { "date",      cmd_date      },
    { "time",      cmd_date      },
    { "whoami",    cmd_whoami    },
    { "hostname",  cmd_hostname  },
    { "banner",    cmd_banner    },
    { "packages",  cmd_packages  },
    { "pkg",       cmd_packages  },
    { "install",   cmd_install   },
    { "uninstall", cmd_uninstall },
    { "remove",    cmd_uninstall },
    { "reboot",    cmd_reboot    },
    { "halt",      cmd_halt      },
    { "shutdown",  cmd_halt      },
    { "panic",     cmd_panic     },
    { NULL,        NULL          }
};

void Commands_Dispatch(ShellCmd *cl)
{
    if (!cl || !cl->cmd[0]) return;

    /* Check built-in commands first */
    for (int i = 0; s_dispatch[i].name; i++) {
        if (cmd_strcmp(cl->cmd, s_dispatch[i].name) == 0) {
            s_dispatch[i].fn(cl);
            return;
        }
    }

    /* Check if it's an installed package name */
    if (PKG_Run(cl->cmd, cl)) return;

    /* Check if it's an uninstalled package */
    int pkg_idx = PKG_Find(cl->cmd);
    if (pkg_idx >= 0) {
        COLOR_SET(VGA_COL_WARNING);
        printf("  Package '%s' is not installed.\n", cl->cmd);
        COLOR_RESET();
        COLOR_SET(VGA_COL_INFO);
        printf("  Install it with: install %s\n", cl->cmd);
        COLOR_RESET();
        return;
    }

    /* Unknown */
    COLOR_SET(VGA_COL_ERROR);
    printf("  Unknown command: '%s'\n", cl->cmd);
    COLOR_RESET();
    cprintln("  Type 'help' to list commands.", VGA_COL_INFO);
}