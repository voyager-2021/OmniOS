/* ============================================================
   OmniOS v1.0.0 - Package Manager + All Packages
   ============================================================ */
#include "shell.h"
#include "../stdio.h"
#include "../arch/i686/vga_text.h"
#include "../arch/i686/io.h"
#include "../drivers/keyboard.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

extern volatile uint32_t g_tick_count;

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

static int pkg_strlen(const char *s)
{
    int n = 0; while (s[n]) n++; return n;
}

static int pkg_atoi(const char *s)
{
    int r = 0, neg = 0;
    if (*s == '-') { neg = 1; s++; }
    while (*s >= '0' && *s <= '9') r = r * 10 + (*s++ - '0');
    return neg ? -r : r;
}

/* ---- Delay helper ---- */
static void pkg_delay(uint32_t ticks)
{
    uint32_t start = g_tick_count;
    while (g_tick_count - start < ticks) __asm__ volatile("hlt");
}

/* ---- PRNG ---- */
static uint32_t pkg_rand_seed = 0;
static uint32_t pkg_rand(void)
{
    if (pkg_rand_seed == 0) pkg_rand_seed = g_tick_count;
    pkg_rand_seed = pkg_rand_seed * 1103515245 + 12345;
    return (pkg_rand_seed >> 16) & 0x7FFF;
}

/* ---- PC Speaker ---- */
static void spk_on(uint32_t freq)
{
    uint32_t div = 1193180 / freq;
    i686_outb(0x43, 0xB6);
    i686_outb(0x42, (uint8_t)(div & 0xFF));
    i686_outb(0x42, (uint8_t)((div >> 8) & 0xFF));
    uint8_t tmp = i686_inb(0x61);
    i686_outb(0x61, tmp | 0x03);
}

static void spk_off(void)
{
    uint8_t tmp = i686_inb(0x61);
    i686_outb(0x61, tmp & 0xFC);
}

/* ============================================================
   PACKAGE: neofetch
   ============================================================ */
static void neofetch_run(ShellCmd *cl)
{
    (void)cl;

    uint32_t ticks = g_tick_count;
    uint32_t secs = ticks / 18;
    uint32_t mins = secs / 60;
    uint32_t hrs = mins / 60;

    int installed = 0;
    for (int i = 0; i < g_package_count; i++)
        if (g_packages[i].installed) installed++;

    /* Logo on left, info on right */
    static const char *logo[] = {
        "     .---.      ",
        "    /     \\     ",
        "   |  O O  |    ",
        "   |  ___  |    ",
        "    \\_____/     ",
        "   /|     |\\    ",
        "  / |     | \\   ",
        " /  |     |  \\  ",
        "    |     |     ",
        "    |_   _|     ",
        "    | | | |     ",
        "    |_| |_|     ",
    };

    static const char *keys[] = {
        "OS",
        "Version",
        "Build",
        "Arch",
        "CPU Mode",
        "Shell",
        "Display",
        "Keyboard",
        "Uptime",
        "Packages",
        "Memory",
        "Disk",
    };

    char uptime_buf[32];
    snprintf(uptime_buf, 32, "%02u:%02u:%02u", hrs, mins % 60, secs % 60);

    char pkg_buf[32];
    snprintf(pkg_buf, 32, "%d/%d installed", installed, g_package_count);

    static const char *vals_static[] = {
        "OmniOS",
        OMNIOS_VERSION,
        OMNIOS_BUILD,
        "x86 (i686) 32-bit",
        "Protected Mode",
        "omni-shell 1.0",
        "VGA Text 80x25",
        "PS/2 IRQ1",
        NULL,  /* filled dynamically */
        NULL,  /* filled dynamically */
        "Extended (see meminfo)",
        "FAT32 250MB",
    };

    const char *vals[12];
    for (int i = 0; i < 12; i++) vals[i] = vals_static[i];
    vals[8] = uptime_buf;
    vals[9] = pkg_buf;

    puts("\n");

    for (int i = 0; i < 12; i++) {
        /* Logo part */
        VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        puts("  ");
        puts(logo[i]);

        /* Info part */
        VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        puts(keys[i]);

        int pad = 12 - pkg_strlen(keys[i]);
        for (int j = 0; j < pad; j++) putc(' ');

        VGA_SetColor(VGA_COL_NORMAL);
        puts(": ");
        puts(vals[i]);
        putc('\n');
    }

    /* Colour palette bar */
    puts("  ");
    puts("                 ");
    for (int c = 0; c < 8; c++) {
        VGA_SetColor(VGA_MAKE_COLOR(c, c));
        puts("   ");
    }
    VGA_SetColor(VGA_COL_NORMAL);
    puts("\n  ");
    puts("                 ");
    for (int c = 8; c < 16; c++) {
        VGA_SetColor(VGA_MAKE_COLOR(c, c));
        puts("   ");
    }
    VGA_SetColor(VGA_COL_NORMAL);
    puts("\n\n");
}

/* ============================================================
   PACKAGE: fortune
   ============================================================ */
static void fortune_run(ShellCmd *cl)
{
    (void)cl;
    static const char *fortunes[] = {
        "The best way to predict the future is to invent it. - Alan Kay",
        "Talk is cheap. Show me the code. - Linus Torvalds",
        "Any sufficiently advanced technology is indistinguishable from magic.",
        "First, solve the problem. Then, write the code. - John Johnson",
        "Simplicity is the soul of efficiency. - Austin Freeman",
        "Make it work, make it right, make it fast. - Kent Beck",
        "Code is like humor. When you have to explain it, it's bad.",
        "Programs must be written for people to read. - Abelson & Sussman",
        "It works on my machine. - Every developer ever",
        "There are only 10 types of people: those who understand binary.",
        "A good programmer looks both ways before crossing a one-way street.",
        "To understand recursion, you must first understand recursion.",
        "99 bugs in the code. Take one down, patch it... 127 bugs in the code.",
        "The best error message is the one that never shows up.",
        "OmniOS: Because every OS starts with a single putc().",
        "In theory, there is no difference between theory and practice.",
    };
    int count = (int)(sizeof(fortunes) / sizeof(fortunes[0]));
    int idx = pkg_rand() % count;

    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    puts("\n  \"");
    puts(fortunes[idx]);
    puts("\"\n\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ============================================================
   PACKAGE: cowsay
   ============================================================ */
static void cowsay_run(ShellCmd *cl)
{
    if (cl->argc < 2) {
        VGA_SetColor(VGA_COL_WARNING);
        puts("  Usage: cowsay <message>\n");
        VGA_SetColor(VGA_COL_NORMAL);
        return;
    }

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

/* ============================================================
   PACKAGE: matrix
   ============================================================ */
static void matrix_run(ShellCmd *cl)
{
    (void)cl;
    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    puts("  Matrix rain... press any key to stop.\n\n");

    for (int frame = 0; frame < 80; frame++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            int r = pkg_rand();
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
        pkg_delay(1);
        if (KB_KeyAvailable()) { KB_TryGetChar(); break; }
    }

    VGA_SetColor(VGA_COL_NORMAL);
    puts("\n\n");
}

/* ============================================================
   PACKAGE: figlet
   ============================================================ */
static void figlet_run(ShellCmd *cl)
{
    if (cl->argc < 2) {
        VGA_SetColor(VGA_COL_WARNING);
        puts("  Usage: figlet <text>\n");
        VGA_SetColor(VGA_COL_NORMAL);
        return;
    }

    VGA_SetColor(VGA_COL_HEADER);
    puts("\n");

    for (int row = 0; row < 5; row++) {
        puts("  ");
        for (int i = 1; i < cl->argc; i++) {
            for (int j = 0; cl->argv[i][j]; j++) {
                char c = cl->argv[i][j];
                if (c >= 'a' && c <= 'z') c -= 32;

                if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
                    switch (row) {
                        case 0: case 4:
                            putc('#'); putc('#'); putc('#'); break;
                        case 1: case 3:
                            putc('#'); putc(' '); putc('#'); break;
                        case 2:
                            if (c == 'A' || c == 'B' || c == 'E' || c == 'F' ||
                                c == 'H' || c == 'P' || c == 'R' || c == 'S' ||
                                c == '8' || c == '0') {
                                putc('#'); putc('#'); putc('#');
                            } else {
                                putc('#'); putc(' '); putc('#');
                            }
                            break;
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

/* ============================================================
   PACKAGE: snake
   ============================================================ */
static void snake_run(ShellCmd *cl)
{
    (void)cl;
    VGA_SetColor(VGA_COL_WARNING);
    puts("  Snake v1.0 - WASD to move, Q to quit\n\n");
    VGA_SetColor(VGA_COL_NORMAL);

    #define FIELD_W 20
    #define FIELD_H 10
    #define MAX_SNAKE 50

    int sx[MAX_SNAKE], sy[MAX_SNAKE];
    int slen = 3;
    int dx = 1, dy = 0;
    int fx, fy;
    int score = 0;
    bool running = true;

    for (int i = 0; i < slen; i++) { sx[i] = 5 - i; sy[i] = 5; }

    fx = pkg_rand() % FIELD_W;
    fy = pkg_rand() % FIELD_H;

    while (running) {
        puts("  ");
        for (int i = 0; i < FIELD_W + 2; i++) putc('-');
        putc('\n');

        for (int y = 0; y < FIELD_H; y++) {
            puts("  |");
            for (int x = 0; x < FIELD_W; x++) {
                bool is_snake = false;
                for (int s = 0; s < slen; s++) {
                    if (sx[s] == x && sy[s] == y) { is_snake = true; break; }
                }
                if (x == sx[0] && y == sy[0]) {
                    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
                    putc('@');
                } else if (is_snake) {
                    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                    putc('o');
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
        printf("  Score: %d  Length: %d\n", score, slen);
        VGA_SetColor(VGA_COL_NORMAL);

        uint32_t deadline = g_tick_count + 4;
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

        int nx = sx[0] + dx, ny = sy[0] + dy;

        if (nx < 0 || nx >= FIELD_W || ny < 0 || ny >= FIELD_H) {
            VGA_SetColor(VGA_COL_ERROR);
            printf("  Game Over! Hit wall. Score: %d\n", score);
            VGA_SetColor(VGA_COL_NORMAL);
            break;
        }

        for (int s = 0; s < slen; s++) {
            if (sx[s] == nx && sy[s] == ny) {
                VGA_SetColor(VGA_COL_ERROR);
                printf("  Game Over! Hit self. Score: %d\n", score);
                VGA_SetColor(VGA_COL_NORMAL);
                running = false; break;
            }
        }
        if (!running) break;

        bool ate = (nx == fx && ny == fy);
        if (!ate) {
            for (int s = slen - 1; s > 0; s--) { sx[s] = sx[s-1]; sy[s] = sy[s-1]; }
        } else {
            if (slen < MAX_SNAKE) {
                for (int s = slen; s > 0; s--) { sx[s] = sx[s-1]; sy[s] = sy[s-1]; }
                slen++;
            }
            score += 10;
            fx = pkg_rand() % FIELD_W;
            fy = pkg_rand() % FIELD_H;
        }
        sx[0] = nx;
        sy[0] = ny;
    }

    #undef FIELD_W
    #undef FIELD_H
    #undef MAX_SNAKE
}

/* ============================================================
   PACKAGE: sysmon
   ============================================================ */
static void sysmon_run(ShellCmd *cl)
{
    (void)cl;
    VGA_SetColor(VGA_COL_HEADER);
    puts("  System Monitor - press Q to exit\n\n");

    bool running = true;
    while (running) {
        uint32_t ticks = g_tick_count;
        uint32_t secs = ticks / 18;
        uint32_t mins = secs / 60;
        uint32_t hrs = mins / 60;

        VGA_SetColor(VGA_COL_INFO);
        printf("  Uptime: %02u:%02u:%02u | Ticks: %-10u | IRQ: active    \r",
               hrs, mins % 60, secs % 60, ticks);

        uint32_t deadline = g_tick_count + 9;
        while (g_tick_count < deadline) {
            char k = KB_TryGetChar();
            if (k == 'q' || k == 'Q') { running = false; break; }
            __asm__ volatile("hlt");
        }
    }

    puts("\n\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ============================================================
   PACKAGE: editor
   ============================================================ */
static void editor_run(ShellCmd *cl)
{
    (void)cl;
    VGA_SetColor(VGA_COL_HEADER);
    puts("  OmniOS Editor v1.0 - ESC to exit\n");
    VGA_DrawHLine('-', VGA_WIDTH, VGA_COL_NORMAL);
    VGA_SetColor(VGA_COL_NORMAL);

    while (1) {
        char c = KB_GetChar();
        if ((unsigned char)c == 0x95) break; /* KB_KEY_ESC */
        if (c == '\b') VGA_PutChar('\b');
        else if (c == '\n' || c == '\r') putc('\n');
        else if ((unsigned char)c >= 0x20 && (unsigned char)c < 0x80) putc(c);
    }

    puts("\n");
    VGA_DrawHLine('-', VGA_WIDTH, VGA_COL_NORMAL);
    VGA_SetColor(VGA_COL_INFO);
    puts("  Editor closed.\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ============================================================
   PACKAGE: beep
   ============================================================ */
static void beep_run(ShellCmd *cl)
{
    int freq = 1000;
    int duration = 3; /* ticks (~165ms) */

    if (cl->argc >= 2) freq = pkg_atoi(cl->argv[1]);
    if (cl->argc >= 3) duration = pkg_atoi(cl->argv[2]);

    if (freq < 20) freq = 20;
    if (freq > 20000) freq = 20000;
    if (duration < 1) duration = 1;
    if (duration > 36) duration = 36; /* max ~2 seconds */

    VGA_SetColor(VGA_COL_INFO);
    printf("  Beep: %d Hz for %d ticks\n", freq, duration);
    VGA_SetColor(VGA_COL_NORMAL);

    spk_on(freq);
    pkg_delay(duration);
    spk_off();
}

/* ============================================================
   PACKAGE: piano
   ============================================================ */
static void piano_run(ShellCmd *cl)
{
    (void)cl;
    VGA_SetColor(VGA_COL_HEADER);
    puts("  OmniOS Piano - press keys to play, Q to quit\n\n");
    VGA_SetColor(VGA_COL_INFO);
    puts("  Keys:  A S D F G H J K  =  C D E F G A B C'\n");
    puts("         W E   T Y U      =  C# D#  F# G# A#\n\n");
    VGA_SetColor(VGA_COL_NORMAL);

    /* Note frequencies */
    static const struct { char key; int freq; const char *name; } notes[] = {
        { 'a', 262, "C4 " }, { 'w', 277, "C#4" },
        { 's', 294, "D4 " }, { 'e', 311, "D#4" },
        { 'd', 330, "E4 " },
        { 'f', 349, "F4 " }, { 't', 370, "F#4" },
        { 'g', 392, "G4 " }, { 'y', 415, "G#4" },
        { 'h', 440, "A4 " }, { 'u', 466, "A#4" },
        { 'j', 494, "B4 " },
        { 'k', 523, "C5 " },
    };
    int num_notes = (int)(sizeof(notes) / sizeof(notes[0]));

    while (1) {
        char c = KB_GetChar();
        if (c == 'q' || c == 'Q') break;

        /* Convert uppercase to lowercase */
        if (c >= 'A' && c <= 'Z') c += 32;

        for (int i = 0; i < num_notes; i++) {
            if (c == notes[i].key) {
                VGA_SetColor(VGA_COL_SUCCESS);
                printf("  %s (%d Hz)\r", notes[i].name, notes[i].freq);
                VGA_SetColor(VGA_COL_NORMAL);
                spk_on(notes[i].freq);
                pkg_delay(3);
                spk_off();
                break;
            }
        }
    }

    spk_off();
    puts("\n  Piano closed.\n");
}

/* ============================================================
   PACKAGE: hangman
   ============================================================ */
static void hangman_run(ShellCmd *cl)
{
    (void)cl;

    static const char *words[] = {
        "kernel", "memory", "driver", "shell", "timer",
        "stack", "queue", "interrupt", "register", "binary",
        "compile", "linker", "assembler", "buffer", "cache",
        "thread", "process", "segment", "bootloader", "pixel",
        "keyboard", "display", "voltage", "circuit", "program",
    };
    int word_count = (int)(sizeof(words) / sizeof(words[0]));

    const char *word = words[pkg_rand() % word_count];
    int wlen = pkg_strlen(word);

    char guessed[32] = {0};
    int  gcount = 0;
    char wrong[16] = {0};
    int  wcount = 0;
    int  max_wrong = 6;

    VGA_SetColor(VGA_COL_HEADER);
    puts("  Hangman - guess the word! (type letters)\n\n");
    VGA_SetColor(VGA_COL_NORMAL);

    while (wcount < max_wrong) {
        /* Draw hangman */
        VGA_SetColor(VGA_COL_NORMAL);
        puts("   +---+\n");
        puts("   |   ");
        putc(wcount >= 1 ? 'O' : ' '); puts("\n");
        puts("   |  ");
        putc(wcount >= 3 ? '/' : ' ');
        putc(wcount >= 2 ? '|' : ' ');
        putc(wcount >= 4 ? '\\' : ' ');
        puts("\n");
        puts("   |  ");
        putc(wcount >= 5 ? '/' : ' ');
        putc(' ');
        putc(wcount >= 6 ? '\\' : ' ');
        puts("\n");
        puts("   +===+\n\n");

        /* Draw word */
        VGA_SetColor(VGA_COL_SUCCESS);
        puts("  Word: ");
        bool complete = true;
        for (int i = 0; i < wlen; i++) {
            bool found = false;
            for (int g = 0; g < gcount; g++) {
                if (guessed[g] == word[i]) { found = true; break; }
            }
            if (found) { putc(word[i]); putc(' '); }
            else { putc('_'); putc(' '); complete = true; complete = false; }
        }

        /* Check if complete */
        complete = true;
        for (int i = 0; i < wlen; i++) {
            bool found = false;
            for (int g = 0; g < gcount; g++) {
                if (guessed[g] == word[i]) { found = true; break; }
            }
            if (!found) { complete = false; break; }
        }

        puts("\n");

        if (complete) {
            VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            printf("\n  You won! The word was: %s\n\n", word);
            VGA_SetColor(VGA_COL_NORMAL);
            return;
        }

        /* Show wrong guesses */
        if (wcount > 0) {
            VGA_SetColor(VGA_COL_ERROR);
            puts("  Wrong: ");
            for (int i = 0; i < wcount; i++) { putc(wrong[i]); putc(' '); }
            puts("\n");
        }

        VGA_SetColor(VGA_COL_INFO);
        printf("  Tries left: %d  |  Guess a letter: ", max_wrong - wcount);
        VGA_SetColor(VGA_COL_NORMAL);

        char c = KB_GetChar();
        if (c >= 'A' && c <= 'Z') c += 32;

        if (c < 'a' || c > 'z') continue;

        putc(c);
        putc('\n');
        putc('\n');

        /* Already guessed? */
        bool dup = false;
        for (int g = 0; g < gcount; g++) {
            if (guessed[g] == c) { dup = true; break; }
        }
        for (int w = 0; w < wcount; w++) {
            if (wrong[w] == c) { dup = true; break; }
        }
        if (dup) {
            VGA_SetColor(VGA_COL_WARNING);
            puts("  Already guessed!\n\n");
            VGA_SetColor(VGA_COL_NORMAL);
            continue;
        }

        /* Check if in word */
        bool in_word = false;
        for (int i = 0; i < wlen; i++) {
            if (word[i] == c) { in_word = true; break; }
        }

        if (in_word) {
            if (gcount < 31) guessed[gcount++] = c;
        } else {
            if (wcount < 15) wrong[wcount++] = c;
        }
    }

    /* Lost */
    VGA_SetColor(VGA_COL_ERROR);
    printf("\n  You lost! The word was: %s\n\n", word);
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ============================================================
   PACKAGE: tictactoe
   ============================================================ */
static void tictactoe_run(ShellCmd *cl)
{
    (void)cl;
    char board[9] = {' ',' ',' ',' ',' ',' ',' ',' ',' '};
    bool player_turn = true;
    int moves = 0;

    VGA_SetColor(VGA_COL_HEADER);
    puts("  Tic-Tac-Toe - You are X, CPU is O\n");
    puts("  Enter position 1-9:\n\n");
    puts("   1 | 2 | 3\n  ---+---+---\n   4 | 5 | 6\n  ---+---+---\n   7 | 8 | 9\n\n");
    VGA_SetColor(VGA_COL_NORMAL);

    /* Check winner helper */
    #define CHECK_WIN(b, c) ( \
        (b[0]==c && b[1]==c && b[2]==c) || \
        (b[3]==c && b[4]==c && b[5]==c) || \
        (b[6]==c && b[7]==c && b[8]==c) || \
        (b[0]==c && b[3]==c && b[6]==c) || \
        (b[1]==c && b[4]==c && b[7]==c) || \
        (b[2]==c && b[5]==c && b[8]==c) || \
        (b[0]==c && b[4]==c && b[8]==c) || \
        (b[2]==c && b[4]==c && b[6]==c)   \
    )

    while (moves < 9) {
        /* Draw board */
        for (int r = 0; r < 3; r++) {
            puts("   ");
            for (int c = 0; c < 3; c++) {
                char ch = board[r*3+c];
                if (ch == 'X') VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                else if (ch == 'O') VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                else VGA_SetColor(VGA_COL_NORMAL);
                putc(' '); putc(ch); putc(' ');
                VGA_SetColor(VGA_COL_NORMAL);
                if (c < 2) putc('|');
            }
            putc('\n');
            if (r < 2) puts("  ---+---+---\n");
        }
        puts("\n");

        if (player_turn) {
            VGA_SetColor(VGA_COL_INFO);
            puts("  Your move (1-9): ");
            VGA_SetColor(VGA_COL_NORMAL);

            char k;
            int pos;
            while (1) {
                k = KB_GetChar();
                if (k >= '1' && k <= '9') {
                    pos = k - '1';
                    if (board[pos] == ' ') break;
                    VGA_SetColor(VGA_COL_WARNING);
                    puts("  Taken! Try again: ");
                    VGA_SetColor(VGA_COL_NORMAL);
                }
            }
            putc(k); putc('\n');
            board[pos] = 'X';
        } else {
            /* Simple AI: try center, then corners, then edges */
            int pos = -1;

            /* Try to win */
            for (int i = 0; i < 9 && pos < 0; i++) {
                if (board[i] == ' ') {
                    board[i] = 'O';
                    if (CHECK_WIN(board, 'O')) pos = i;
                    board[i] = ' ';
                }
            }
            /* Block player */
            for (int i = 0; i < 9 && pos < 0; i++) {
                if (board[i] == ' ') {
                    board[i] = 'X';
                    if (CHECK_WIN(board, 'X')) pos = i;
                    board[i] = ' ';
                }
            }
            /* Center */
            if (pos < 0 && board[4] == ' ') pos = 4;
            /* Corners */
            int corners[] = {0,2,6,8};
            for (int i = 0; i < 4 && pos < 0; i++)
                if (board[corners[i]] == ' ') pos = corners[i];
            /* Any */
            for (int i = 0; i < 9 && pos < 0; i++)
                if (board[i] == ' ') pos = i;

            board[pos] = 'O';
            VGA_SetColor(VGA_COL_INFO);
            printf("  CPU plays position %d\n\n", pos + 1);
            VGA_SetColor(VGA_COL_NORMAL);
        }

        moves++;

        if (CHECK_WIN(board, 'X')) {
            VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            puts("  You win!\n\n");
            VGA_SetColor(VGA_COL_NORMAL);
            return;
        }
        if (CHECK_WIN(board, 'O')) {
            VGA_SetColor(VGA_COL_ERROR);
            puts("  CPU wins!\n\n");
            VGA_SetColor(VGA_COL_NORMAL);
            return;
        }

        player_turn = !player_turn;
    }

    VGA_SetColor(VGA_COL_WARNING);
    puts("  It's a draw!\n\n");
    VGA_SetColor(VGA_COL_NORMAL);

    #undef CHECK_WIN
}

/* ============================================================
   PACKAGE: mandelbrot
   ============================================================ */
static void mandelbrot_run(ShellCmd *cl)
{
    (void)cl;
    VGA_SetColor(VGA_COL_HEADER);
    puts("  Mandelbrot Set (ASCII)\n\n");
    VGA_SetColor(VGA_COL_NORMAL);

    /* Fixed-point: 16.16 */
    #define FP_SHIFT 12
    #define FP_ONE   (1 << FP_SHIFT)
    #define FP_MUL(a,b) (((a) * (b)) >> FP_SHIFT)

    static const char shades[] = " .:-=+*#%@";
    int max_iter = 10;

    for (int row = 0; row < 22; row++) {
        for (int col = 0; col < 78; col++) {
            /* Map to complex plane: real -2.5..1.0, imag -1.1..1.1 */
            int cr = -2 * FP_ONE + (col * 3 * FP_ONE) / 78;
            int ci = -1 * FP_ONE + (row * 2 * FP_ONE) / 22;

            int zr = 0, zi = 0;
            int iter = 0;

            while (iter < max_iter) {
                int zr2 = FP_MUL(zr, zr);
                int zi2 = FP_MUL(zi, zi);
                if (zr2 + zi2 > 4 * FP_ONE) break;
                int new_zr = zr2 - zi2 + cr;
                zi = 2 * FP_MUL(zr, zi) + ci;
                zr = new_zr;
                iter++;
            }

            int shade_idx = (iter * 9) / max_iter;
            if (shade_idx > 9) shade_idx = 9;

            uint8_t colors[] = {
                VGA_COLOR_BLACK, VGA_COLOR_BLUE, VGA_COLOR_GREEN,
                VGA_COLOR_CYAN, VGA_COLOR_RED, VGA_COLOR_MAGENTA,
                VGA_COLOR_BROWN, VGA_COLOR_LIGHT_GREY, VGA_COLOR_WHITE,
                VGA_COLOR_YELLOW
            };
            VGA_SetColor(VGA_MAKE_COLOR(colors[shade_idx], VGA_COLOR_BLACK));
            putc(shades[shade_idx]);
        }
        putc('\n');
    }

    #undef FP_SHIFT
    #undef FP_ONE
    #undef FP_MUL

    puts("\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ============================================================
   PACKAGE: clock
   ============================================================ */
static void clock_run(ShellCmd *cl)
{
    (void)cl;
    VGA_SetColor(VGA_COL_HEADER);
    puts("  Digital Clock - press Q to exit\n\n");

    /* Big digit patterns (3x5 each) */
    static const char *digits[10][5] = {
        { "###", "# #", "# #", "# #", "###" }, /* 0 */
        { "  #", "  #", "  #", "  #", "  #" }, /* 1 */
        { "###", "  #", "###", "#  ", "###" }, /* 2 */
        { "###", "  #", "###", "  #", "###" }, /* 3 */
        { "# #", "# #", "###", "  #", "  #" }, /* 4 */
        { "###", "#  ", "###", "  #", "###" }, /* 5 */
        { "###", "#  ", "###", "# #", "###" }, /* 6 */
        { "###", "  #", "  #", "  #", "  #" }, /* 7 */
        { "###", "# #", "###", "# #", "###" }, /* 8 */
        { "###", "# #", "###", "  #", "###" }, /* 9 */
    };

    while (1) {
        uint32_t secs = g_tick_count / 18;
        uint32_t mins = secs / 60;
        uint32_t hrs = mins / 60;

        int time_digits[6] = {
            (hrs / 10) % 10, hrs % 10,
            (mins % 60) / 10, (mins % 60) % 10,
            (secs % 60) / 10, (secs % 60) % 10,
        };

        for (int row = 0; row < 5; row++) {
            puts("     ");
            for (int d = 0; d < 6; d++) {
                VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                puts(digits[time_digits[d]][row]);
                VGA_SetColor(VGA_COL_NORMAL);
                if (d == 1 || d == 3) {
                    if (row == 1 || row == 3) puts(" : ");
                    else puts("   ");
                } else {
                    putc(' ');
                }
            }
            putc('\n');
        }

        puts("\n");

        uint32_t deadline = g_tick_count + 18; /* ~1 second */
        bool quit = false;
        while (g_tick_count < deadline) {
            char k = KB_TryGetChar();
            if (k == 'q' || k == 'Q') { quit = true; break; }
            __asm__ volatile("hlt");
        }
        if (quit) break;

        /* Move cursor up to redraw */
        /* Since we can't easily move up, just continue scrolling */
    }

    VGA_SetColor(VGA_COL_NORMAL);
    puts("  Clock stopped.\n");
}

/* ============================================================
   PACKAGE: passwords
   ============================================================ */
static void passwords_run(ShellCmd *cl)
{
    int length = 16;
    int count = 5;

    if (cl->argc >= 2) length = pkg_atoi(cl->argv[1]);
    if (cl->argc >= 3) count = pkg_atoi(cl->argv[2]);

    if (length < 4) length = 4;
    if (length > 64) length = 64;
    if (count < 1) count = 1;
    if (count > 20) count = 20;

    static const char charset[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "!@#$%^&*()-_=+[]{}|;:,.<>?";
    int clen = pkg_strlen(charset);

    VGA_SetColor(VGA_COL_HEADER);
    printf("  Generated %d passwords (length %d):\n\n", count, length);

    for (int p = 0; p < count; p++) {
        VGA_SetColor(VGA_COL_INFO);
        printf("  %2d: ", p + 1);
        VGA_SetColor(VGA_COL_SUCCESS);
        for (int i = 0; i < length; i++) {
            putc(charset[pkg_rand() % clen]);
        }
        putc('\n');
    }

    puts("\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ============================================================
   PACKAGE: rot13
   ============================================================ */
static void rot13_run(ShellCmd *cl)
{
    if (cl->argc < 2) {
        VGA_SetColor(VGA_COL_WARNING);
        puts("  Usage: rot13 <text>\n");
        VGA_SetColor(VGA_COL_NORMAL);
        return;
    }

    VGA_SetColor(VGA_COL_SUCCESS);
    puts("  ");
    for (int i = 1; i < cl->argc; i++) {
        for (int j = 0; cl->argv[i][j]; j++) {
            char c = cl->argv[i][j];
            if (c >= 'a' && c <= 'z') c = 'a' + (c - 'a' + 13) % 26;
            else if (c >= 'A' && c <= 'Z') c = 'A' + (c - 'A' + 13) % 26;
            putc(c);
        }
        if (i < cl->argc - 1) putc(' ');
    }
    puts("\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ============================================================
   PACKAGE: wc (word count)
   ============================================================ */
static void wc_run(ShellCmd *cl)
{
    if (cl->argc < 2) {
        VGA_SetColor(VGA_COL_WARNING);
        puts("  Usage: wc <text>\n");
        VGA_SetColor(VGA_COL_NORMAL);
        return;
    }

    int chars = 0, words = cl->argc - 1;
    for (int i = 1; i < cl->argc; i++) {
        for (int j = 0; cl->argv[i][j]; j++) chars++;
        if (i < cl->argc - 1) chars++; /* space */
    }

    VGA_SetColor(VGA_COL_INFO);
    printf("  Words: %d  Characters: %d\n", words, chars);
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ============================================================
   PACKAGE: rev (reverse text)
   ============================================================ */
static void rev_run(ShellCmd *cl)
{
    if (cl->argc < 2) {
        VGA_SetColor(VGA_COL_WARNING);
        puts("  Usage: rev <text>\n");
        VGA_SetColor(VGA_COL_NORMAL);
        return;
    }

    /* Build full string */
    static char buf[256];
    int pos = 0;
    for (int i = 1; i < cl->argc && pos < 254; i++) {
        for (int j = 0; cl->argv[i][j] && pos < 254; j++)
            buf[pos++] = cl->argv[i][j];
        if (i < cl->argc - 1 && pos < 254) buf[pos++] = ' ';
    }
    buf[pos] = '\0';

    VGA_SetColor(VGA_COL_SUCCESS);
    puts("  ");
    for (int i = pos - 1; i >= 0; i--) putc(buf[i]);
    puts("\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ============================================================
   PACKAGE: rand
   ============================================================ */
static void rand_run(ShellCmd *cl)
{
    int min_val = 1, max_val = 100;

    if (cl->argc >= 2) max_val = pkg_atoi(cl->argv[1]);
    if (cl->argc >= 3) { min_val = pkg_atoi(cl->argv[1]); max_val = pkg_atoi(cl->argv[2]); }

    if (min_val > max_val) { int t = min_val; min_val = max_val; max_val = t; }

    int range = max_val - min_val + 1;
    int result = min_val + (int)(pkg_rand() % range);

    VGA_SetColor(VGA_COL_SUCCESS);
    printf("  Random [%d-%d]: %d\n", min_val, max_val, result);
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ============================================================
   PACKAGE: sleep
   ============================================================ */
static void sleep_run(ShellCmd *cl)
{
    int seconds = 1;
    if (cl->argc >= 2) seconds = pkg_atoi(cl->argv[1]);
    if (seconds < 1) seconds = 1;
    if (seconds > 60) seconds = 60;

    VGA_SetColor(VGA_COL_INFO);
    printf("  Sleeping for %d second(s)...\n", seconds);
    VGA_SetColor(VGA_COL_NORMAL);

    pkg_delay(seconds * 18);

    VGA_SetColor(VGA_COL_SUCCESS);
    puts("  Done.\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ============================================================
   Package Registry Init
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

    /* ---- Utilities ---- */
    pkg_add("neofetch",    "System info with ASCII art",      "1.0", neofetch_run);
    pkg_add("fortune",     "Random programming quotes",       "1.0", fortune_run);
    pkg_add("cowsay",      "ASCII cow says your message",     "1.0", cowsay_run);
    pkg_add("figlet",      "Big block text renderer",         "1.0", figlet_run);
    pkg_add("rot13",       "ROT13 cipher encode/decode",      "1.0", rot13_run);
    pkg_add("wc",          "Word and character count",         "1.0", wc_run);
    pkg_add("rev",         "Reverse text",                     "1.0", rev_run);
    pkg_add("rand",        "Random number generator",          "1.0", rand_run);
    pkg_add("sleep",       "Sleep for N seconds",              "1.0", sleep_run);
    pkg_add("passwords",   "Generate random passwords",        "1.0", passwords_run);

    /* ---- Tools ---- */
    pkg_add("editor",      "Simple text editor (ESC quit)",    "1.0", editor_run);
    pkg_add("sysmon",      "Live system monitor (Q quit)",     "1.0", sysmon_run);
    pkg_add("clock",       "Large digital clock (Q quit)",     "1.0", clock_run);

    /* ---- Audio ---- */
    pkg_add("beep",        "PC speaker beep [freq] [ticks]",   "1.0", beep_run);
    pkg_add("piano",       "Musical keyboard (Q quit)",        "1.0", piano_run);

    /* ---- Visual ---- */
    pkg_add("matrix",      "Matrix digital rain effect",       "1.0", matrix_run);
    pkg_add("mandelbrot",  "ASCII Mandelbrot fractal",         "1.0", mandelbrot_run);

    /* ---- Games ---- */
    pkg_add("snake",       "Classic snake game (WASD+Q)",      "1.0", snake_run);
    pkg_add("hangman",     "Word guessing game",               "1.0", hangman_run);
    pkg_add("tictactoe",   "Tic-Tac-Toe vs CPU",              "1.0", tictactoe_run);
}