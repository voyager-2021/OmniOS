/* ============================================================
   OmniOS v1.0.0 - Package Manager
   Built-in packages that can be "installed" and "uninstalled".
   ============================================================ */
#include "shell.h"
#include "../stdio.h"
#include "../arch/i686/vga_text.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ---- String helpers ---- */
static int pkg_strcmp(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

static void pkg_strcpy(char *d, const char *s)
{
    while (*s) *d++ = *s++;
    *d = '\0';
}

/* ============================================================
   Package implementations
   ============================================================ */

/* ---- fortune: random quotes ---- */
static void fortune_run(ShellCmd *cl)
{
    (void)cl;
    extern volatile uint32_t g_tick_count;

    static const char *fortunes[] = {
        "The best way to predict the future is to invent it. - Alan Kay",
        "Talk is cheap. Show me the code. - Linus Torvalds",
        "Any sufficiently advanced technology is indistinguishable from magic. - Arthur C. Clarke",
        "First, solve the problem. Then, write the code. - John Johnson",
        "The computer was born to solve problems that did not exist before. - Bill Gates",
        "Simplicity is the soul of efficiency. - Austin Freeman",
        "Make it work, make it right, make it fast. - Kent Beck",
        "Code is like humor. When you have to explain it, it's bad. - Cory House",
        "Programs must be written for people to read. - Abelson & Sussman",
        "It works on my machine. - Every developer ever",
        "There are only 10 types of people: those who understand binary and those who don't.",
        "A good programmer looks both ways before crossing a one-way street.",
        "To understand recursion, you must first understand recursion.",
        "99 little bugs in the code, 99 bugs in the code. Take one down, patch it around... 127 bugs in the code.",
        "The best error message is the one that never shows up. - Thomas Fuchs",
        "OmniOS: Because every OS starts with a single putc().",
    };
    int count = (int)(sizeof(fortunes) / sizeof(fortunes[0]));
    int idx = g_tick_count % count;

    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    puts("\n  ");
    puts(fortunes[idx]);
    puts("\n\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ---- cowsay: ASCII cow ---- */
static void cowsay_run(ShellCmd *cl)
{
    if (cl->argc < 2) {
        VGA_SetColor(VGA_COL_WARNING);
        puts("  Usage: cowsay <message>\n");
        VGA_SetColor(VGA_COL_NORMAL);
        return;
    }

    /* Build message */
    static char msg[200];
    int pos = 0;
    for (int i = 1; i < cl->argc && pos < 198; i++) {
        for (int j = 0; cl->argv[i][j] && pos < 198; j++)
            msg[pos++] = cl->argv[i][j];
        if (i < cl->argc - 1 && pos < 198) msg[pos++] = ' ';
    }
    msg[pos] = '\0';

    int len = pos;

    VGA_SetColor(VGA_COL_NORMAL);
    puts("   ");
    for (int i = 0; i < len + 2; i++) putc('_');
    puts("\n  < ");
    puts(msg);
    puts(" >\n   ");
    for (int i = 0; i < len + 2; i++) putc('-');
    puts("\n");
    puts("          \\   ^__^\n");
    puts("           \\  (oo)\\_______\n");
    puts("              (__)\\       )\\/\\\n");
    puts("                  ||----w |\n");
    puts("                  ||     ||\n");
}

/* ---- matrix: matrix rain effect ---- */
static void matrix_run(ShellCmd *cl)
{
    (void)cl;
    extern volatile uint32_t g_tick_count;

    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    puts("  Starting Matrix rain... (watch for ~3 seconds)\n\n");

    uint32_t seed = g_tick_count;
    for (int frame = 0; frame < 60; frame++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            seed = seed * 1103515245 + 12345;
            int r = (seed >> 16) & 0x7FFF;
            if (r % 3 == 0) {
                char c = '!' + (r % 94);
                VGA_SetColor(VGA_MAKE_COLOR(
                    (r % 5 == 0) ? VGA_COLOR_WHITE : VGA_COLOR_LIGHT_GREEN,
                    VGA_COLOR_BLACK));
                putc(c);
            } else {
                putc(' ');
            }
        }
        /* tiny delay */
        uint32_t t = g_tick_count;
        while (g_tick_count - t < 1) __asm__ volatile("hlt");
    }

    VGA_SetColor(VGA_COL_NORMAL);
    puts("\n\n  Matrix effect ended.\n");
}

/* ---- figlet: big text ---- */
static void figlet_run(ShellCmd *cl)
{
    if (cl->argc < 2) {
        VGA_SetColor(VGA_COL_WARNING);
        puts("  Usage: figlet <text>\n");
        VGA_SetColor(VGA_COL_NORMAL);
        return;
    }

    /* Simple block letter renderer for A-Z, 0-9 */
    VGA_SetColor(VGA_COL_HEADER);
    puts("\n");

    for (int row = 0; row < 5; row++) {
        puts("  ");
        for (int i = 1; i < cl->argc; i++) {
            for (int j = 0; cl->argv[i][j]; j++) {
                char c = cl->argv[i][j];
                if (c >= 'a' && c <= 'z') c -= 32;

                /* Each letter is 5 rows x 5 cols using # and space */
                /* Simplified: just make block letters */
                if (c >= 'A' && c <= 'Z') {
                    int idx = c - 'A';
                    /* Simple pattern: top, sides, middle, sides, bottom */
                    switch (row) {
                        case 0: case 4:
                            putc('#'); putc('#'); putc('#'); break;
                        case 1: case 3:
                            putc('#'); putc(' '); putc('#'); break;
                        case 2:
                            if (c == 'A' || c == 'B' || c == 'E' || c == 'F' ||
                                c == 'H' || c == 'P' || c == 'R' || c == 'S') {
                                putc('#'); putc('#'); putc('#');
                            } else {
                                putc('#'); putc(' '); putc('#');
                            }
                            break;
                    }
                } else if (c >= '0' && c <= '9') {
                    switch (row) {
                        case 0: case 4:
                            putc('#'); putc('#'); putc('#'); break;
                        case 2:
                            putc('#'); putc('#'); putc('#'); break;
                        default:
                            putc('#'); putc(' '); putc('#'); break;
                    }
                } else if (c == ' ') {
                    putc(' '); putc(' '); putc(' ');
                } else {
                    putc(c); putc(' '); putc(' ');
                }
                putc(' ');
            }
            putc(' ');
        }
        putc('\n');
    }

    puts("\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ---- snake: tiny snake game ---- */
static void snake_run(ShellCmd *cl)
{
    (void)cl;
    VGA_SetColor(VGA_COL_WARNING);
    puts("  Snake game v0.1\n");
    puts("  Use W/A/S/D to move, Q to quit.\n\n");
    VGA_SetColor(VGA_COL_NORMAL);

    extern volatile uint32_t g_tick_count;

    /* Simple 20x10 field */
    #define FIELD_W 20
    #define FIELD_H 10
    #define MAX_SNAKE 50

    int sx[MAX_SNAKE], sy[MAX_SNAKE];
    int slen = 3;
    int dx = 1, dy = 0;
    int fx, fy;
    int score = 0;
    bool running = true;

    /* Init snake */
    for (int i = 0; i < slen; i++) {
        sx[i] = 5 - i;
        sy[i] = 5;
    }

    /* Place food */
    uint32_t seed = g_tick_count;
    seed = seed * 1103515245 + 12345;
    fx = (seed >> 16) % FIELD_W;
    seed = seed * 1103515245 + 12345;
    fy = (seed >> 16) % FIELD_H;

    while (running) {
        /* Draw field */
        VGA_SaveState();
        int cx, cy;
        VGA_GetCursorPos(&cx, &cy);

        /* Move cursor to draw area */
        /* We'll just print and scroll naturally */
        puts("  ");
        for (int i = 0; i < FIELD_W + 2; i++) putc('-');
        putc('\n');

        for (int y = 0; y < FIELD_H; y++) {
            puts("  |");
            for (int x = 0; x < FIELD_W; x++) {
                bool is_snake = false;
                for (int s = 0; s < slen; s++) {
                    if (sx[s] == x && sy[s] == y) {
                        is_snake = true;
                        break;
                    }
                }
                if (is_snake) {
                    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                    putc('O');
                } else if (x == fx && y == fy) {
                    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                    putc('*');
                } else {
                    VGA_SetColor(VGA_COL_NORMAL);
                    putc(' ');
                }
            }
            VGA_SetColor(VGA_COL_NORMAL);
            puts("|\n");
        }
        puts("  ");
        for (int i = 0; i < FIELD_W + 2; i++) putc('-');
        putc('\n');

        VGA_SetColor(VGA_COL_INFO);
        printf("  Score: %d  |  WASD=move  Q=quit\n", score);
        VGA_SetColor(VGA_COL_NORMAL);

        /* Wait for input with timeout */
        uint32_t deadline = g_tick_count + 4; /* ~220ms */
        while (g_tick_count < deadline) {
            char k = KB_TryGetChar();
            if (k == 'w' || k == 'W') { if (dy != 1)  { dx = 0; dy = -1; } break; }
            if (k == 's' || k == 'S') { if (dy != -1) { dx = 0; dy = 1;  } break; }
            if (k == 'a' || k == 'A') { if (dx != 1)  { dx = -1; dy = 0; } break; }
            if (k == 'd' || k == 'D') { if (dx != -1) { dx = 1;  dy = 0; } break; }
            if (k == 'q' || k == 'Q') { running = false; break; }
            __asm__ volatile("hlt");
        }

        if (!running) break;

        /* Move snake */
        int new_x = sx[0] + dx;
        int new_y = sy[0] + dy;

        /* Wall collision */
        if (new_x < 0 || new_x >= FIELD_W || new_y < 0 || new_y >= FIELD_H) {
            VGA_SetColor(VGA_COL_ERROR);
            printf("  Game Over! Hit wall. Final score: %d\n", score);
            VGA_SetColor(VGA_COL_NORMAL);
            break;
        }

        /* Self collision */
        for (int s = 0; s < slen; s++) {
            if (sx[s] == new_x && sy[s] == new_y) {
                VGA_SetColor(VGA_COL_ERROR);
                printf("  Game Over! Hit self. Final score: %d\n", score);
                VGA_SetColor(VGA_COL_NORMAL);
                running = false;
                break;
            }
        }
        if (!running) break;

        /* Check food */
        bool ate = (new_x == fx && new_y == fy);

        /* Shift body */
        if (!ate) {
            for (int s = slen - 1; s > 0; s--) {
                sx[s] = sx[s - 1];
                sy[s] = sy[s - 1];
            }
        } else {
            if (slen < MAX_SNAKE) {
                for (int s = slen; s > 0; s--) {
                    sx[s] = sx[s - 1];
                    sy[s] = sy[s - 1];
                }
                slen++;
            }
            score += 10;

            /* New food */
            seed = seed * 1103515245 + 12345;
            fx = (seed >> 16) % FIELD_W;
            seed = seed * 1103515245 + 12345;
            fy = (seed >> 16) % FIELD_H;
        }

        sx[0] = new_x;
        sy[0] = new_y;
    }
}

/* ---- sysmon: system monitor ---- */
static void sysmon_run(ShellCmd *cl)
{
    (void)cl;
    extern volatile uint32_t g_tick_count;

    VGA_SetColor(VGA_COL_HEADER);
    puts("  System Monitor (press Q to exit)\n\n");

    bool running = true;
    while (running) {
        VGA_SetColor(VGA_COL_INFO);
        uint32_t ticks = g_tick_count;
        uint32_t secs = ticks / 18;
        uint32_t mins = secs / 60;
        uint32_t hrs = mins / 60;

        printf("  Uptime: %02u:%02u:%02u  |  Ticks: %u      \r",
               hrs, mins % 60, secs % 60, ticks);

        /* Check for Q to exit */
        uint32_t deadline = g_tick_count + 9; /* ~500ms update */
        while (g_tick_count < deadline) {
            char k = KB_TryGetChar();
            if (k == 'q' || k == 'Q') { running = false; break; }
            __asm__ volatile("hlt");
        }
    }

    puts("\n\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ---- texteditor: simple text viewer/editor ---- */
static void texteditor_run(ShellCmd *cl)
{
    (void)cl;
    VGA_SetColor(VGA_COL_HEADER);
    puts("  OmniOS Text Editor v0.1\n");
    VGA_SetColor(VGA_COL_INFO);
    puts("  Type text freely. Press ESC to exit.\n");
    VGA_DrawHLine('-', VGA_WIDTH, VGA_COL_NORMAL);

    extern volatile uint32_t g_tick_count;
    (void)g_tick_count;

    VGA_SetColor(VGA_COL_NORMAL);

    while (1) {
        char c = KB_GetChar();
        if ((unsigned char)c == 0x95) break; /* KB_KEY_ESC */
        if (c == '\b') {
            VGA_PutChar('\b');
        } else if (c == '\n' || c == '\r') {
            putc('\n');
        } else if ((unsigned char)c >= 0x20 && (unsigned char)c < 0x80) {
            putc(c);
        }
    }

    puts("\n");
    VGA_DrawHLine('-', VGA_WIDTH, VGA_COL_NORMAL);
    VGA_SetColor(VGA_COL_INFO);
    puts("  Editor closed.\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ============================================================
   Package Registry
   ============================================================ */

Package g_packages[PKG_MAX_PACKAGES];
int     g_package_count = 0;

static void pkg_add(const char *name, const char *desc, const char *ver,
                    void (*on_run)(ShellCmd *))
{
    if (g_package_count >= PKG_MAX_PACKAGES) return;
    Package *p = &g_packages[g_package_count++];
    pkg_strcpy(p->name, name);
    pkg_strcpy(p->description, desc);
    pkg_strcpy(p->version, ver);
    p->installed = false;
    p->on_install = 0;
    p->on_run = on_run;
}

void PKG_Init(void)
{
    g_package_count = 0;
    pkg_add("fortune",    "Random programming quotes",     "1.0", fortune_run);
    pkg_add("cowsay",     "ASCII cow says your message",   "1.0", cowsay_run);
    pkg_add("matrix",     "Matrix digital rain effect",    "1.0", matrix_run);
    pkg_add("figlet",     "Big block text renderer",       "1.0", figlet_run);
    pkg_add("snake",      "Classic snake game (WASD+Q)",   "1.0", snake_run);
    pkg_add("sysmon",     "Live system monitor (Q quit)",  "1.0", sysmon_run);
    pkg_add("editor",     "Simple text editor (ESC quit)", "1.0", texteditor_run);
}

int PKG_Find(const char *name)
{
    for (int i = 0; i < g_package_count; i++) {
        if (pkg_strcmp(g_packages[i].name, name) == 0) return i;
    }
    return -1;
}

bool PKG_Install(const char *name)
{
    int idx = PKG_Find(name);
    if (idx < 0) return false;
    if (g_packages[idx].installed) return true;
    g_packages[idx].installed = true;
    if (g_packages[idx].on_install) g_packages[idx].on_install();
    return true;
}

bool PKG_Uninstall(const char *name)
{
    int idx = PKG_Find(name);
    if (idx < 0) return false;
    g_packages[idx].installed = false;
    return true;
}

bool PKG_IsInstalled(const char *name)
{
    int idx = PKG_Find(name);
    if (idx < 0) return false;
    return g_packages[idx].installed;
}

bool PKG_Run(const char *name, ShellCmd *cl)
{
    int idx = PKG_Find(name);
    if (idx < 0) return false;
    if (!g_packages[idx].installed) return false;
    if (!g_packages[idx].on_run) return false;
    g_packages[idx].on_run(cl);
    return true;
}