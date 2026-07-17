/* ============================================================
   OmniOS v1.0.0 - Commands + Package Manager + All Packages
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

extern volatile uint32_t g_tick_count;

#define COLOR_SET(c)   VGA_SetColor(c)
#define COLOR_RESET()  VGA_SetColor(VGA_COL_NORMAL)

/* ============================================================
   String helpers
   ============================================================ */
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

static void cmd_strcpy(char *d, const char *s)
{
    while (*s) *d++ = *s++;
    *d = '\0';
}

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
   Package delay + PRNG + PC speaker
   ============================================================ */
static void pkg_delay(uint32_t ticks)
{
    uint32_t start = g_tick_count;
    while (g_tick_count - start < ticks) __asm__ volatile("hlt");
}

static uint32_t pkg_rand_seed = 0;
static uint32_t pkg_rand(void)
{
    if (pkg_rand_seed == 0) pkg_rand_seed = g_tick_count;
    pkg_rand_seed = pkg_rand_seed * 1103515245 + 12345;
    return (pkg_rand_seed >> 16) & 0x7FFF;
}

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
   Package registry
   ============================================================ */
Package g_packages[PKG_MAX_PACKAGES];
int     g_package_count = 0;

static void pkg_add(const char *name, const char *desc, const char *ver,
                    void (*on_run)(ShellCmd *))
{
    if (g_package_count >= PKG_MAX_PACKAGES) return;
    Package *p = &g_packages[g_package_count++];
    cmd_strcpy(p->name, name);
    cmd_strcpy(p->description, desc);
    cmd_strcpy(p->version, ver);
    p->installed = false;
    p->on_install = 0;
    p->on_run = on_run;
}

int PKG_Find(const char *name)
{
    for (int i = 0; i < g_package_count; i++)
        if (cmd_strcmp(g_packages[i].name, name) == 0) return i;
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
        "OS", "Version", "Build", "Arch", "CPU Mode", "Shell",
        "Display", "Keyboard", "Uptime", "Packages", "Memory", "Disk",
    };

    char uptime_buf[32];
    snprintf(uptime_buf, 32, "%02u:%02u:%02u", hrs, mins % 60, secs % 60);

    char pkg_buf[32];
    snprintf(pkg_buf, 32, "%d/%d installed", installed, g_package_count);

    static const char *vals_static[] = {
        "OmniOS", OMNIOS_VERSION, OMNIOS_BUILD, "x86 (i686) 32-bit",
        "Protected Mode", "omni-shell 1.0", "VGA Text 80x25", "PS/2 IRQ1",
        NULL, NULL, "Extended (see meminfo)", "FAT32 250MB",
    };

    const char *vals[12];
    for (int i = 0; i < 12; i++) vals[i] = vals_static[i];
    vals[8] = uptime_buf;
    vals[9] = pkg_buf;

    puts("\n");
    for (int i = 0; i < 12; i++) {
        VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        puts("  ");
        puts(logo[i]);

        VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        puts(keys[i]);
        int pad = 12 - cmd_strlen(keys[i]);
        for (int j = 0; j < pad; j++) putc(' ');

        VGA_SetColor(VGA_COL_NORMAL);
        puts(": ");
        puts(vals[i]);
        putc('\n');
    }

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
        "99 bugs in the code. Take one down, patch it... 127 bugs in the code.",
        "OmniOS: Because every OS starts with a single putc().",
    };
    int count = (int)(sizeof(fortunes) / sizeof(fortunes[0]));
    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    puts("\n  \"");
    puts(fortunes[pkg_rand() % count]);
    puts("\"\n\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ============================================================
   PACKAGE: cowsay
   ============================================================ */
static void cowsay_run(ShellCmd *cl)
{
    if (cl->argc < 2) {
        cprintln("  Usage: cowsay <message>", VGA_COL_WARNING);
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

    VGA_SetColor(VGA_COL_NORMAL);
    puts("   ");
    for (int i = 0; i < pos + 2; i++) putc('_');
    puts("\n  < "); puts(msg); puts(" >\n   ");
    for (int i = 0; i < pos + 2; i++) putc('-');
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
                VGA_SetColor(VGA_MAKE_COLOR(
                    (r % 5 == 0) ? VGA_COLOR_WHITE : VGA_COLOR_LIGHT_GREEN,
                    VGA_COLOR_BLACK));
                putc('!' + (r % 94));
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
        cprintln("  Usage: figlet <text>", VGA_COL_WARNING);
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
                            if (c=='A'||c=='B'||c=='E'||c=='F'||c=='H'||c=='P'||c=='R'||c=='S'||c=='8'||c=='0')
                                { putc('#'); putc('#'); putc('#'); }
                            else { putc('#'); putc(' '); putc('#'); }
                            break;
                    }
                } else if (c == ' ') { puts("   "); }
                else { putc(c); puts("  "); }
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
    cprintln("  Snake v1.0 - WASD to move, Q to quit\n", VGA_COL_WARNING);

    #define FIELD_W 20
    #define FIELD_H 10
    #define MAX_SNAKE 50

    int sx[MAX_SNAKE], sy[MAX_SNAKE];
    int slen = 3, dx = 1, dy = 0, fx, fy, score = 0;
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
                for (int s = 0; s < slen; s++)
                    if (sx[s] == x && sy[s] == y) { is_snake = true; break; }
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
        COLOR_SET(VGA_COL_INFO);
        printf("  Score: %d  Length: %d\n", score, slen);
        COLOR_RESET();

        uint32_t deadline = g_tick_count + 4;
        while (g_tick_count < deadline) {
            char k = KB_TryGetChar();
            if (k=='w'||k=='W') { if(dy!=1)  {dx=0;dy=-1;} break; }
            if (k=='s'||k=='S') { if(dy!=-1) {dx=0;dy=1;}  break; }
            if (k=='a'||k=='A') { if(dx!=1)  {dx=-1;dy=0;} break; }
            if (k=='d'||k=='D') { if(dx!=-1) {dx=1;dy=0;}  break; }
            if (k=='q'||k=='Q') { running=false; break; }
            __asm__ volatile("hlt");
        }
        if (!running) break;

        int nx = sx[0]+dx, ny = sy[0]+dy;
        if (nx<0||nx>=FIELD_W||ny<0||ny>=FIELD_H) {
            COLOR_SET(VGA_COL_ERROR);
            printf("  Game Over! Hit wall. Score: %d\n", score);
            COLOR_RESET(); break;
        }
        for (int s = 0; s < slen; s++) {
            if (sx[s]==nx && sy[s]==ny) {
                COLOR_SET(VGA_COL_ERROR);
                printf("  Game Over! Hit self. Score: %d\n", score);
                COLOR_RESET(); running=false; break;
            }
        }
        if (!running) break;

        bool ate = (nx==fx && ny==fy);
        if (!ate) {
            for (int s=slen-1;s>0;s--) {sx[s]=sx[s-1];sy[s]=sy[s-1];}
        } else {
            if (slen<MAX_SNAKE) {
                for (int s=slen;s>0;s--) {sx[s]=sx[s-1];sy[s]=sy[s-1];}
                slen++;
            }
            score+=10;
            fx=pkg_rand()%FIELD_W; fy=pkg_rand()%FIELD_H;
        }
        sx[0]=nx; sy[0]=ny;
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
    cprintln("  System Monitor - press Q to exit\n", VGA_COL_HEADER);
    bool running = true;
    while (running) {
        uint32_t ticks = g_tick_count;
        uint32_t secs = ticks/18, mins = secs/60, hrs = mins/60;
        COLOR_SET(VGA_COL_INFO);
        printf("  Uptime: %02u:%02u:%02u | Ticks: %-10u\r", hrs, mins%60, secs%60, ticks);
        uint32_t deadline = g_tick_count + 9;
        while (g_tick_count < deadline) {
            char k = KB_TryGetChar();
            if (k=='q'||k=='Q') { running=false; break; }
            __asm__ volatile("hlt");
        }
    }
    puts("\n\n");
    COLOR_RESET();
}

/* ============================================================
   PACKAGE: editor
   ============================================================ */
static void editor_run(ShellCmd *cl)
{
    (void)cl;
    cprintln("  OmniOS Editor v1.0 - ESC to exit", VGA_COL_HEADER);
    VGA_DrawHLine('-', VGA_WIDTH, VGA_COL_NORMAL);
    COLOR_RESET();
    while (1) {
        char c = KB_GetChar();
        if ((unsigned char)c == 0x95) break;
        if (c == '\b') VGA_PutChar('\b');
        else if (c == '\n' || c == '\r') putc('\n');
        else if ((unsigned char)c >= 0x20 && (unsigned char)c < 0x80) putc(c);
    }
    puts("\n");
    VGA_DrawHLine('-', VGA_WIDTH, VGA_COL_NORMAL);
    cprintln("  Editor closed.", VGA_COL_INFO);
}

/* ============================================================
   PACKAGE: beep
   ============================================================ */
static void beep_run(ShellCmd *cl)
{
    int freq = 1000, duration = 3;
    if (cl->argc >= 2) freq = cmd_atoi(cl->argv[1]);
    if (cl->argc >= 3) duration = cmd_atoi(cl->argv[2]);
    if (freq < 20) freq = 20;
    if (freq > 20000) freq = 20000;
    if (duration < 1) duration = 1;
    if (duration > 36) duration = 36;
    COLOR_SET(VGA_COL_INFO);
    printf("  Beep: %d Hz for %d ticks\n", freq, duration);
    COLOR_RESET();
    spk_on(freq); pkg_delay(duration); spk_off();
}

/* ============================================================
   PACKAGE: piano
   ============================================================ */
static void piano_run(ShellCmd *cl)
{
    (void)cl;
    cprintln("  OmniOS Piano - press keys to play, Q to quit\n", VGA_COL_HEADER);
    COLOR_SET(VGA_COL_INFO);
    puts("  Keys:  A S D F G H J K  =  C D E F G A B C'\n");
    puts("         W E   T Y U      =  C# D#  F# G# A#\n\n");
    COLOR_RESET();

    static const struct { char key; int freq; const char *name; } notes[] = {
        {'a',262,"C4"},{'w',277,"C#"},{'s',294,"D4"},{'e',311,"D#"},
        {'d',330,"E4"},{'f',349,"F4"},{'t',370,"F#"},{'g',392,"G4"},
        {'y',415,"G#"},{'h',440,"A4"},{'u',466,"A#"},{'j',494,"B4"},
        {'k',523,"C5"},
    };
    int num = (int)(sizeof(notes)/sizeof(notes[0]));

    while (1) {
        char c = KB_GetChar();
        if (c=='q'||c=='Q') break;
        if (c>='A'&&c<='Z') c+=32;
        for (int i = 0; i < num; i++) {
            if (c == notes[i].key) {
                COLOR_SET(VGA_COL_SUCCESS);
                printf("  %s (%d Hz)  \r", notes[i].name, notes[i].freq);
                COLOR_RESET();
                spk_on(notes[i].freq); pkg_delay(3); spk_off();
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
        "kernel","memory","driver","shell","timer","stack","queue",
        "interrupt","register","binary","compile","linker","buffer",
        "cache","thread","process","segment","bootloader","pixel",
        "keyboard","display","program",
    };
    int word_count = (int)(sizeof(words)/sizeof(words[0]));
    const char *word = words[pkg_rand() % word_count];
    int wlen = cmd_strlen(word);

    char guessed[32]={0}; int gcount=0;
    char wrong[16]={0};   int wcount=0;
    int max_wrong = 6;

    cprintln("  Hangman - guess the word!\n", VGA_COL_HEADER);

    while (wcount < max_wrong) {
        puts("   +---+\n   |   ");
        putc(wcount>=1?'O':' '); puts("\n   |  ");
        putc(wcount>=3?'/':' '); putc(wcount>=2?'|':' '); putc(wcount>=4?'\\':' ');
        puts("\n   |  ");
        putc(wcount>=5?'/':' '); putc(' '); putc(wcount>=6?'\\':' ');
        puts("\n   +===+\n\n");

        COLOR_SET(VGA_COL_SUCCESS);
        puts("  Word: ");
        bool complete = true;
        for (int i = 0; i < wlen; i++) {
            bool found = false;
            for (int g = 0; g < gcount; g++)
                if (guessed[g]==word[i]) { found=true; break; }
            if (found) { putc(word[i]); putc(' '); }
            else { putc('_'); putc(' '); complete = false; }
        }
        puts("\n");

        if (complete) {
            VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            printf("\n  You won! The word was: %s\n\n", word);
            COLOR_RESET(); return;
        }

        if (wcount > 0) {
            COLOR_SET(VGA_COL_ERROR);
            puts("  Wrong: ");
            for (int i = 0; i < wcount; i++) { putc(wrong[i]); putc(' '); }
            puts("\n");
        }

        COLOR_SET(VGA_COL_INFO);
        printf("  Tries left: %d  Guess: ", max_wrong - wcount);
        COLOR_RESET();

        char c = KB_GetChar();
        if (c>='A'&&c<='Z') c+=32;
        if (c<'a'||c>'z') continue;
        putc(c); puts("\n\n");

        bool dup = false;
        for (int g=0;g<gcount;g++) if(guessed[g]==c) {dup=true;break;}
        for (int w=0;w<wcount;w++) if(wrong[w]==c)    {dup=true;break;}
        if (dup) { cprintln("  Already guessed!\n", VGA_COL_WARNING); continue; }

        bool in_word = false;
        for (int i=0;i<wlen;i++) if(word[i]==c) {in_word=true;break;}
        if (in_word) { if(gcount<31) guessed[gcount++]=c; }
        else         { if(wcount<15) wrong[wcount++]=c;   }
    }

    COLOR_SET(VGA_COL_ERROR);
    printf("\n  You lost! The word was: %s\n\n", word);
    COLOR_RESET();
}

/* ============================================================
   PACKAGE: tictactoe
   ============================================================ */
static void tictactoe_run(ShellCmd *cl)
{
    (void)cl;
    char board[9]={' ',' ',' ',' ',' ',' ',' ',' ',' '};
    bool player_turn = true;
    int moves = 0;

    cprintln("  Tic-Tac-Toe - You=X, CPU=O. Enter 1-9:\n", VGA_COL_HEADER);

    #define CHECK_WIN(b,c) ( \
        (b[0]==c&&b[1]==c&&b[2]==c)||(b[3]==c&&b[4]==c&&b[5]==c)|| \
        (b[6]==c&&b[7]==c&&b[8]==c)||(b[0]==c&&b[3]==c&&b[6]==c)|| \
        (b[1]==c&&b[4]==c&&b[7]==c)||(b[2]==c&&b[5]==c&&b[8]==c)|| \
        (b[0]==c&&b[4]==c&&b[8]==c)||(b[2]==c&&b[4]==c&&b[6]==c))

    while (moves < 9) {
        for (int r=0;r<3;r++) {
            puts("   ");
            for (int c=0;c<3;c++) {
                char ch=board[r*3+c];
                if(ch=='X') VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN,VGA_COLOR_BLACK));
                else if(ch=='O') VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_RED,VGA_COLOR_BLACK));
                else VGA_SetColor(VGA_COL_NORMAL);
                putc(' '); putc(ch); putc(' ');
                VGA_SetColor(VGA_COL_NORMAL);
                if(c<2) putc('|');
            }
            putc('\n');
            if(r<2) puts("  ---+---+---\n");
        }
        puts("\n");

        if (player_turn) {
            COLOR_SET(VGA_COL_INFO); puts("  Your move (1-9): "); COLOR_RESET();
            char k; int pos;
            while(1) {
                k=KB_GetChar();
                if(k>='1'&&k<='9') { pos=k-'1'; if(board[pos]==' ') break; }
            }
            putc(k); putc('\n');
            board[pos]='X';
        } else {
            int pos=-1;
            for(int i=0;i<9&&pos<0;i++){if(board[i]==' '){board[i]='O';if(CHECK_WIN(board,'O'))pos=i;board[i]=' ';}}
            for(int i=0;i<9&&pos<0;i++){if(board[i]==' '){board[i]='X';if(CHECK_WIN(board,'X'))pos=i;board[i]=' ';}}
            if(pos<0&&board[4]==' ') pos=4;
            int corners[]={0,2,6,8};
            for(int i=0;i<4&&pos<0;i++) if(board[corners[i]]==' ') pos=corners[i];
            for(int i=0;i<9&&pos<0;i++) if(board[i]==' ') pos=i;
            board[pos]='O';
            COLOR_SET(VGA_COL_INFO);
            printf("  CPU plays %d\n\n", pos+1);
            COLOR_RESET();
        }
        moves++;

        if(CHECK_WIN(board,'X')){cprintln("  You win!\n",VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN,VGA_COLOR_BLACK));return;}
        if(CHECK_WIN(board,'O')){cprintln("  CPU wins!\n",VGA_COL_ERROR);return;}
        player_turn=!player_turn;
    }
    cprintln("  It's a draw!\n", VGA_COL_WARNING);
    #undef CHECK_WIN
}

/* ============================================================
   PACKAGE: mandelbrot
   ============================================================ */
static void mandelbrot_run(ShellCmd *cl)
{
    (void)cl;
    cprintln("  Mandelbrot Set (ASCII)\n", VGA_COL_HEADER);

    #define FP_SHIFT 12
    #define FP_ONE   (1 << FP_SHIFT)
    #define FP_MUL(a,b) (((a)*(b))>>FP_SHIFT)

    static const char shades[]=" .:-=+*#%@";
    int max_iter=10;
    uint8_t colors[]={0,1,2,3,4,5,6,7,15,14};

    for(int row=0;row<22;row++){
        for(int col=0;col<78;col++){
            int cr=-2*FP_ONE+(col*3*FP_ONE)/78;
            int ci=-1*FP_ONE+(row*2*FP_ONE)/22;
            int zr=0,zi=0,iter=0;
            while(iter<max_iter){
                int zr2=FP_MUL(zr,zr),zi2=FP_MUL(zi,zi);
                if(zr2+zi2>4*FP_ONE)break;
                int new_zr=zr2-zi2+cr;
                zi=2*FP_MUL(zr,zi)+ci; zr=new_zr; iter++;
            }
            int si=(iter*9)/max_iter; if(si>9)si=9;
            VGA_SetColor(VGA_MAKE_COLOR(colors[si],VGA_COLOR_BLACK));
            putc(shades[si]);
        }
        putc('\n');
    }
    #undef FP_SHIFT
    #undef FP_ONE
    #undef FP_MUL
    puts("\n");
    COLOR_RESET();
}

/* ============================================================
   PACKAGE: clock
   ============================================================ */
static void clock_run(ShellCmd *cl)
{
    (void)cl;
    cprintln("  Digital Clock - Q to exit\n", VGA_COL_HEADER);

    static const char *digits[10][5]={
        {"###","# #","# #","# #","###"},{"  #","  #","  #","  #","  #"},
        {"###","  #","###","#  ","###"},{"###","  #","###","  #","###"},
        {"# #","# #","###","  #","  #"},{"###","#  ","###","  #","###"},
        {"###","#  ","###","# #","###"},{"###","  #","  #","  #","  #"},
        {"###","# #","###","# #","###"},{"###","# #","###","  #","###"},
    };

    while(1){
        uint32_t secs=g_tick_count/18,mins=secs/60,hrs=mins/60;
        int td[6]={(hrs/10)%10,hrs%10,(mins%60)/10,(mins%60)%10,(secs%60)/10,(secs%60)%10};
        for(int row=0;row<5;row++){
            puts("     ");
            for(int d=0;d<6;d++){
                VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN,VGA_COLOR_BLACK));
                puts(digits[td[d]][row]);
                COLOR_RESET();
                if(d==1||d==3){if(row==1||row==3)puts(" : ");else puts("   ");}
                else putc(' ');
            }
            putc('\n');
        }
        puts("\n");

        uint32_t deadline=g_tick_count+18;
        bool quit=false;
        while(g_tick_count<deadline){
            char k=KB_TryGetChar();
            if(k=='q'||k=='Q'){quit=true;break;}
            __asm__ volatile("hlt");
        }
        if(quit)break;
    }
    COLOR_RESET();
    puts("  Clock stopped.\n");
}

/* ============================================================
   PACKAGE: passwords
   ============================================================ */
static void passwords_run(ShellCmd *cl)
{
    int length=16, count=5;
    if(cl->argc>=2) length=cmd_atoi(cl->argv[1]);
    if(cl->argc>=3) count=cmd_atoi(cl->argv[2]);
    if(length<4)length=4; if(length>64)length=64;
    if(count<1)count=1; if(count>20)count=20;

    static const char charset[]=
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+";
    int clen=cmd_strlen(charset);

    COLOR_SET(VGA_COL_HEADER);
    printf("  %d passwords (length %d):\n\n", count, length);
    for(int p=0;p<count;p++){
        COLOR_SET(VGA_COL_INFO); printf("  %2d: ",p+1);
        COLOR_SET(VGA_COL_SUCCESS);
        for(int i=0;i<length;i++) putc(charset[pkg_rand()%clen]);
        putc('\n');
    }
    puts("\n"); COLOR_RESET();
}

/* ============================================================
   PACKAGE: rot13
   ============================================================ */
static void rot13_run(ShellCmd *cl)
{
    if(cl->argc<2){cprintln("  Usage: rot13 <text>",VGA_COL_WARNING);return;}
    COLOR_SET(VGA_COL_SUCCESS);
    puts("  ");
    for(int i=1;i<cl->argc;i++){
        for(int j=0;cl->argv[i][j];j++){
            char c=cl->argv[i][j];
            if(c>='a'&&c<='z') c='a'+(c-'a'+13)%26;
            else if(c>='A'&&c<='Z') c='A'+(c-'A'+13)%26;
            putc(c);
        }
        if(i<cl->argc-1) putc(' ');
    }
    puts("\n"); COLOR_RESET();
}

/* ============================================================
   PACKAGE: wc
   ============================================================ */
static void wc_run(ShellCmd *cl)
{
    if(cl->argc<2){cprintln("  Usage: wc <text>",VGA_COL_WARNING);return;}
    int chars=0, words=cl->argc-1;
    for(int i=1;i<cl->argc;i++){
        for(int j=0;cl->argv[i][j];j++) chars++;
        if(i<cl->argc-1) chars++;
    }
    COLOR_SET(VGA_COL_INFO);
    printf("  Words: %d  Characters: %d\n", words, chars);
    COLOR_RESET();
}

/* ============================================================
   PACKAGE: rev
   ============================================================ */
static void rev_run(ShellCmd *cl)
{
    if(cl->argc<2){cprintln("  Usage: rev <text>",VGA_COL_WARNING);return;}
    static char buf[256]; int pos=0;
    for(int i=1;i<cl->argc&&pos<254;i++){
        for(int j=0;cl->argv[i][j]&&pos<254;j++) buf[pos++]=cl->argv[i][j];
        if(i<cl->argc-1&&pos<254) buf[pos++]=' ';
    }
    buf[pos]='\0';
    COLOR_SET(VGA_COL_SUCCESS);
    puts("  ");
    for(int i=pos-1;i>=0;i--) putc(buf[i]);
    puts("\n"); COLOR_RESET();
}

/* ============================================================
   PACKAGE: rand
   ============================================================ */
static void rand_run(ShellCmd *cl)
{
    int lo=1,hi=100;
    if(cl->argc>=2) hi=cmd_atoi(cl->argv[1]);
    if(cl->argc>=3){lo=cmd_atoi(cl->argv[1]);hi=cmd_atoi(cl->argv[2]);}
    if(lo>hi){int t=lo;lo=hi;hi=t;}
    int range=hi-lo+1;
    int result=lo+(int)(pkg_rand()%range);
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  Random [%d-%d]: %d\n", lo, hi, result);
    COLOR_RESET();
}

/* ============================================================
   PACKAGE: sleep
   ============================================================ */
static void sleep_run(ShellCmd *cl)
{
    int seconds=1;
    if(cl->argc>=2) seconds=cmd_atoi(cl->argv[1]);
    if(seconds<1)seconds=1; if(seconds>60)seconds=60;
    COLOR_SET(VGA_COL_INFO);
    printf("  Sleeping %d second(s)...\n", seconds);
    COLOR_RESET();
    pkg_delay(seconds*18);
    cprintln("  Done.", VGA_COL_SUCCESS);
}

/* ============================================================
   PKG_Init - register all packages
   ============================================================ */
void PKG_Init(void)
{
    g_package_count = 0;
    pkg_add("neofetch",   "System info with ASCII art",     "1.0", neofetch_run);
    pkg_add("fortune",    "Random programming quotes",      "1.0", fortune_run);
    pkg_add("cowsay",     "ASCII cow says your message",    "1.0", cowsay_run);
    pkg_add("figlet",     "Big block text renderer",        "1.0", figlet_run);
    pkg_add("rot13",      "ROT13 cipher encode/decode",     "1.0", rot13_run);
    pkg_add("wc",         "Word and character count",        "1.0", wc_run);
    pkg_add("rev",        "Reverse text",                    "1.0", rev_run);
    pkg_add("rand",       "Random number generator",         "1.0", rand_run);
    pkg_add("sleep",      "Sleep for N seconds",             "1.0", sleep_run);
    pkg_add("passwords",  "Generate random passwords",       "1.0", passwords_run);
    pkg_add("editor",     "Simple text editor (ESC quit)",   "1.0", editor_run);
    pkg_add("sysmon",     "Live system monitor (Q quit)",    "1.0", sysmon_run);
    pkg_add("clock",      "Large digital clock (Q quit)",    "1.0", clock_run);
    pkg_add("beep",       "PC speaker beep [freq] [ticks]",  "1.0", beep_run);
    pkg_add("piano",      "Musical keyboard (Q quit)",       "1.0", piano_run);
    pkg_add("matrix",     "Matrix digital rain effect",      "1.0", matrix_run);
    pkg_add("mandelbrot", "ASCII Mandelbrot fractal",        "1.0", mandelbrot_run);
    pkg_add("snake",      "Classic snake game (WASD+Q)",     "1.0", snake_run);
    pkg_add("hangman",    "Word guessing game",              "1.0", hangman_run);
    pkg_add("tictactoe",  "Tic-Tac-Toe vs CPU",             "1.0", tictactoe_run);
}

/* ============================================================
   Built-in commands
   ============================================================ */
static void cmd_help(ShellCmd *cl)
{
    (void)cl;
    typedef struct { const char *name; const char *desc; } E;
    static const E cmds[] = {
        {"help",             "Show this help message"},
        {"clear / cls",      "Clear the terminal screen"},
        {"echo <text>",      "Print text to terminal"},
        {"version",          "Show OmniOS version"},
        {"sysinfo",          "Display system information"},
        {"meminfo",          "Show memory layout"},
        {"cpuinfo",          "Show CPU / arch information"},
        {"history",          "List command history"},
        {"color <0-7>",      "Change colour scheme"},
        {"calc <expr>",      "Integer calculator (e.g. calc 10+3)"},
        {"ascii",            "Print ASCII table"},
        {"hex <addr> [len]", "Hex dump memory"},
        {"peek <addr>",      "Read byte from address"},
        {"poke <addr> <v>",  "Write byte to address"},
        {"uptime",           "Show tick counter"},
        {"date",             "Show system date/time"},
        {"whoami",           "Show current user"},
        {"hostname [name]",  "Show/set hostname"},
        {"banner",           "Redraw boot banner"},
        {"packages / pkg",   "List available packages"},
        {"install <pkg>",    "Install a package (or 'all')"},
        {"uninstall <pkg>",  "Uninstall a package (or 'all')"},
        {"reboot",           "Reboot the system"},
        {"halt / shutdown",  "Halt the CPU"},
        {"panic",            "Trigger kernel panic (test)"},
        {NULL,NULL}
    };
    hr('=',VGA_WIDTH,VGA_COL_HEADER);
    cprintln("   OmniOS v" OMNIOS_VERSION " Command Reference", VGA_COL_HEADER);
    hr('=',VGA_WIDTH,VGA_COL_HEADER);
    for(int i=0;cmds[i].name;i++){
        COLOR_SET(VGA_COL_SUCCESS);
        puts("  "); puts(cmds[i].name);
        int pad=26-cmd_strlen(cmds[i].name);
        for(int j=0;j<pad;j++) putc(' ');
        COLOR_RESET();
        puts(cmds[i].desc); putc('\n');
    }
    hr('-',VGA_WIDTH,VGA_COL_HEADER);
    cprintln("  Installed packages also work as commands.", VGA_COL_INFO);
    hr('-',VGA_WIDTH,VGA_COL_HEADER);
}

static void cmd_clear(ShellCmd *cl) { (void)cl; VGA_ClearScreen(); }

static void cmd_echo(ShellCmd *cl)
{
    for(int i=1;i<cl->argc;i++){
        puts(cl->argv[i]);
        if(i<cl->argc-1) putc(' ');
    }
    putc('\n');
}

static void cmd_version(ShellCmd *cl)
{
    (void)cl;
    cprint("  OmniOS v",VGA_COL_INFO);
    cprint(OMNIOS_VERSION,VGA_COL_SUCCESS);
    cprint("  build ",VGA_COL_INFO);
    cprintln(OMNIOS_BUILD,VGA_COL_SUCCESS);
}

static void cmd_sysinfo(ShellCmd *cl)
{
    (void)cl;
    hr('-',VGA_WIDTH,VGA_COL_HEADER);
    cprintln("   OmniOS System Information",VGA_COL_HEADER);
    hr('-',VGA_WIDTH,VGA_COL_HEADER);
    kv_row("OS Name","OmniOS",16);
    kv_row("Version",OMNIOS_VERSION,16);
    kv_row("Build",OMNIOS_BUILD,16);
    kv_row("Architecture","x86 (i686) 32-bit",16);
    kv_row("CPU Mode","Protected Mode",16);
    kv_row("Bootloader","Stage1 (MBR) + Stage2 ELF",16);
    kv_row("Filesystem","FAT12/FAT16/FAT32",16);
    kv_row("Display","VGA Text 80x25 @ 0xB8000",16);
    kv_row("Keyboard","PS/2 IRQ1 ring buffer",16);
    kv_row("Build System","SCons + i686-elf-gcc",16);
    int installed=0;
    for(int i=0;i<g_package_count;i++) if(g_packages[i].installed) installed++;
    COLOR_SET(VGA_COL_INFO);
    printf("  Packages        : %d/%d installed\n",installed,g_package_count);
    COLOR_RESET();
    hr('-',VGA_WIDTH,VGA_COL_HEADER);
}

static void cmd_meminfo(ShellCmd *cl)
{
    (void)cl;
    hr('-',VGA_WIDTH,VGA_COL_HEADER);
    cprintln("   Memory Layout",VGA_COL_HEADER);
    hr('-',VGA_WIDTH,VGA_COL_HEADER);
    cprintln("  0x00000-0x003FF  IVT",VGA_COL_NORMAL);
    cprintln("  0x00400-0x004FF  BDA",VGA_COL_NORMAL);
    cprintln("  0x07C00-0x07DFF  Stage1 MBR",VGA_COL_NORMAL);
    cprintln("  0x20000-0x2FFFF  Stage2 bootloader",VGA_COL_NORMAL);
    cprintln("  0xB8000-0xBFFFF  VGA framebuffer",VGA_COL_NORMAL);
    cprintln("  0x100000+        Kernel",VGA_COL_NORMAL);
    hr('-',VGA_WIDTH,VGA_COL_HEADER);
}

static void cmd_cpuinfo(ShellCmd *cl)
{
    (void)cl;
    hr('-',VGA_WIDTH,VGA_COL_HEADER);
    cprintln("   CPU Information",VGA_COL_HEADER);
    hr('-',VGA_WIDTH,VGA_COL_HEADER);
    kv_row("Architecture","x86 (IA-32 / i686)",16);
    kv_row("Mode","32-bit Protected Mode",16);
    kv_row("GDT","3 segments loaded",16);
    kv_row("IDT","256 entries",16);
    kv_row("PIC","i8259A remapped 0x20-0x2F",16);
    kv_row("Paging","Disabled (flat model)",16);
    hr('-',VGA_WIDTH,VGA_COL_HEADER);
}

static void cmd_history(ShellCmd *cl)
{
    (void)cl;
    cprintln("  Command History:",VGA_COL_HEADER);
    if(s_hist_count==0){cprintln("  (empty)",VGA_COL_WARNING);return;}
    for(int i=s_hist_count;i>=1;i--){
        const char *h=history_get(i);
        if(!h)continue;
        COLOR_SET(VGA_COL_INFO);
        printf("  %3d  ",s_hist_count-i+1);
        COLOR_RESET();
        puts(h); putc('\n');
    }
}

static void cmd_color(ShellCmd *cl)
{
    typedef struct{const char *name;uint8_t col;}S;
    static const S schemes[]={
        {"Classic (grey/black)",  VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREY,VGA_COLOR_BLACK)},
        {"Matrix  (green/black)", VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN,VGA_COLOR_BLACK)},
        {"Ocean   (cyan/black)",  VGA_MAKE_COLOR(VGA_COLOR_LIGHT_CYAN,VGA_COLOR_BLACK)},
        {"Sunset  (red/black)",   VGA_MAKE_COLOR(VGA_COLOR_LIGHT_RED,VGA_COLOR_BLACK)},
        {"Royal   (blue/black)",  VGA_MAKE_COLOR(VGA_COLOR_LIGHT_BLUE,VGA_COLOR_BLACK)},
        {"Gold    (yellow/black)",VGA_MAKE_COLOR(VGA_COLOR_YELLOW,VGA_COLOR_BLACK)},
        {"Mint    (white/blue)",  VGA_MAKE_COLOR(VGA_COLOR_WHITE,VGA_COLOR_BLUE)},
        {"Alert   (white/red)",   VGA_MAKE_COLOR(VGA_COLOR_WHITE,VGA_COLOR_RED)},
    };
    int num=(int)(sizeof(schemes)/sizeof(schemes[0]));
    if(cl->argc<2){
        cprintln("  Colour schemes:",VGA_COL_HEADER);
        for(int i=0;i<num;i++){COLOR_SET(schemes[i].col);printf("  %d  %s\n",i,schemes[i].name);}
        COLOR_RESET();
        cprintln("\n  Usage: color <0-7>",VGA_COL_WARNING);
        return;
    }
    int n=cmd_atoi(cl->argv[1]);
    if(n<0||n>=num){cprintln("  Invalid scheme.",VGA_COL_ERROR);return;}
    VGA_SetColor(schemes[n].col);
    VGA_ClearScreen();
    printf("  Colour scheme %d applied.\n",n);
}

static void cmd_calc(ShellCmd *cl)
{
    if(cl->argc<2){cprintln("  Usage: calc <expr>",VGA_COL_WARNING);return;}
    static char expr[128]; int ep=0;
    for(int i=1;i<cl->argc&&ep<126;i++)
        for(int j=0;cl->argv[i][j]&&ep<126;j++) expr[ep++]=cl->argv[i][j];
    expr[ep]='\0';

    char *p=expr;
    bool neg_a=0;
    if(*p=='-'){neg_a=1;p++;} else if(*p=='+')p++;
    int64_t a=0;
    while(*p>='0'&&*p<='9') a=a*10+(*p++-'0');
    if(neg_a) a=-a;

    char op=*p++;
    if(!op){COLOR_SET(VGA_COL_SUCCESS);printf("  = %d\n",(int)a);COLOR_RESET();return;}

    bool neg_b=0;
    if(*p=='-'){neg_b=1;p++;} else if(*p=='+')p++;
    int64_t b=0;
    while(*p>='0'&&*p<='9') b=b*10+(*p++-'0');
    if(neg_b) b=-b;

    int64_t result=0;
    switch(op){
        case '+':result=a+b;break;
        case '-':result=a-b;break;
        case '*':case 'x':result=a*b;break;
        case '/':if(b==0){cprintln("  Div by zero",VGA_COL_ERROR);return;}result=a/b;break;
        case '%':if(b==0){cprintln("  Mod by zero",VGA_COL_ERROR);return;}result=a%b;break;
        default:COLOR_SET(VGA_COL_ERROR);printf("  Unknown op '%c'\n",op);COLOR_RESET();return;
    }
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  %d %c %d = %d\n",(int)a,op,(int)b,(int)result);
    COLOR_RESET();
}

static void cmd_ascii(ShellCmd *cl)
{
    (void)cl;
    cprintln("  ASCII Table (32-126):",VGA_COL_HEADER);
    int col=0;
    for(int i=32;i<=126;i++){
        COLOR_SET(VGA_COL_INFO);printf(" %3d",i);
        COLOR_RESET();printf(" %c  ",(char)i);
        col++; if(col%10==0)putc('\n');
    }
    if(col%10!=0)putc('\n');
}

static void cmd_hex(ShellCmd *cl)
{
    if(cl->argc<2){cprintln("  Usage: hex <addr> [len]",VGA_COL_WARNING);return;}
    uintptr_t addr=parse_addr(cl->argv[1]);
    int len=(cl->argc>=3)?cmd_atoi(cl->argv[2]):128;
    if(len<=0||len>1024)len=128;
    const uint8_t *mem=(const uint8_t*)addr;
    COLOR_SET(VGA_COL_HEADER);printf("  Hex @ 0x%08x (%d bytes)\n",(unsigned)addr,len);COLOR_RESET();
    hr('-',VGA_WIDTH,VGA_COL_NORMAL);
    for(int row=0;row<len;row+=16){
        COLOR_SET(VGA_COL_INFO);printf("  %08x  ",(unsigned)(addr+row));COLOR_RESET();
        for(int b=0;b<16;b++){
            if(row+b<len)printf("%02x ",mem[row+b]);else puts("   ");
            if(b==7)putc(' ');
        }
        puts(" |");
        COLOR_SET(VGA_COL_SUCCESS);
        for(int b=0;b<16&&row+b<len;b++){
            uint8_t byte=mem[row+b];
            putc((byte>=0x20&&byte<0x7F)?(char)byte:'.');
        }
        COLOR_RESET(); puts("|\n");
    }
}

static void cmd_peek(ShellCmd *cl)
{
    if(cl->argc<2){cprintln("  Usage: peek <addr>",VGA_COL_WARNING);return;}
    uintptr_t addr=parse_addr(cl->argv[1]);
    uint8_t val=*(volatile uint8_t*)addr;
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  [0x%08x] = 0x%02x (%d) '%c'\n",(unsigned)addr,val,val,
           (val>=0x20&&val<0x7F)?(char)val:'.');
    COLOR_RESET();
}

static void cmd_poke(ShellCmd *cl)
{
    if(cl->argc<3){cprintln("  Usage: poke <addr> <val>",VGA_COL_WARNING);return;}
    uintptr_t addr=parse_addr(cl->argv[1]);
    uint8_t val=(uint8_t)cmd_atoi(cl->argv[2]);
    *(volatile uint8_t*)addr=val;
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  Wrote 0x%02x to [0x%08x]\n",val,(unsigned)addr);
    COLOR_RESET();
}

static void cmd_uptime(ShellCmd *cl)
{
    (void)cl;
    uint32_t ticks=g_tick_count,secs=ticks/18,mins=secs/60,hrs=mins/60;
    COLOR_SET(VGA_COL_INFO);
    printf("  Uptime: %02u:%02u:%02u (%u ticks)\n",hrs,mins%60,secs%60,ticks);
    COLOR_RESET();
}

static void cmd_date(ShellCmd *cl)
{
    (void)cl;
    uint32_t secs=g_tick_count/18,mins=secs/60,hrs=mins/60;
    COLOR_SET(VGA_COL_INFO);
    printf("  System time: %02u:%02u:%02u (since boot)\n",hrs,mins%60,secs%60);
    COLOR_RESET();
    cprintln("  Note: No RTC driver. Time relative to boot.",VGA_COL_WARNING);
}

static char g_hostname[32]="omnios";

static void cmd_whoami(ShellCmd *cl)
{
    (void)cl;
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  root@%s\n",g_hostname);
    COLOR_RESET();
}

static void cmd_hostname(ShellCmd *cl)
{
    if(cl->argc>=2){
        int i=0; const char *s=cl->argv[1];
        while(*s&&i<30) g_hostname[i++]=*s++;
        g_hostname[i]='\0';
        COLOR_SET(VGA_COL_SUCCESS);
        printf("  Hostname set to: %s\n",g_hostname);
        COLOR_RESET();
    } else {
        COLOR_SET(VGA_COL_INFO);
        printf("  Hostname: %s\n",g_hostname);
        COLOR_RESET();
    }
}

static void cmd_banner(ShellCmd *cl)
{
    (void)cl;
    VGA_ClearScreen();
    hr('*',VGA_WIDTH,VGA_COL_HEADER);
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
    printf("   Terminal OS  v%s  |  Build %s\n",OMNIOS_VERSION,OMNIOS_BUILD);
    COLOR_RESET();
    puts("\n");
    hr('*',VGA_WIDTH,VGA_COL_HEADER);
    cprintln("  Type 'help' for commands.",VGA_COL_INFO);
}

static void cmd_packages(ShellCmd *cl)
{
    (void)cl;
    hr('=',VGA_WIDTH,VGA_COL_HEADER);
    cprintln("   OmniOS Package Manager",VGA_COL_HEADER);
    hr('=',VGA_WIDTH,VGA_COL_HEADER);
    if(g_package_count==0){cprintln("  No packages.",VGA_COL_WARNING);return;}
    COLOR_SET(VGA_COL_INFO);
    printf("  %-16s %-8s %-10s %s\n","PACKAGE","VER","STATUS","DESCRIPTION");
    COLOR_RESET();
    hr('-',VGA_WIDTH,VGA_COL_NORMAL);
    for(int i=0;i<g_package_count;i++){
        Package *p=&g_packages[i];
        if(p->installed){
            COLOR_SET(VGA_COL_SUCCESS);
            printf("  %-16s %-8s ",p->name,p->version);
            cprint("[installed]",VGA_COL_SUCCESS);
        } else {
            COLOR_SET(VGA_COL_NORMAL);
            printf("  %-16s %-8s ",p->name,p->version);
            cprint("[  ready  ]",VGA_COL_WARNING);
        }
        COLOR_SET(VGA_COL_NORMAL);
        printf(" %s\n",p->description);
    }
    hr('-',VGA_WIDTH,VGA_COL_NORMAL);
    cprintln("  install <name> | install all | uninstall <name>",VGA_COL_INFO);
}

static void cmd_install(ShellCmd *cl)
{
    if(cl->argc<2){cprintln("  Usage: install <pkg>",VGA_COL_WARNING);return;}
    const char *name=cl->argv[1];
    if(cmd_strcmp(name,"all")==0){
        int count=0;
        for(int i=0;i<g_package_count;i++){
            if(!g_packages[i].installed){g_packages[i].installed=true;
                COLOR_SET(VGA_COL_SUCCESS);
                printf("  Installed: %s v%s\n",g_packages[i].name,g_packages[i].version);
                COLOR_RESET(); count++;
            }
        }
        if(count==0)cprintln("  All already installed.",VGA_COL_INFO);
        else{COLOR_SET(VGA_COL_SUCCESS);printf("  %d package(s) installed.\n",count);COLOR_RESET();}
        return;
    }
    int idx=PKG_Find(name);
    if(idx<0){COLOR_SET(VGA_COL_ERROR);printf("  Package '%s' not found.\n",name);COLOR_RESET();return;}
    if(g_packages[idx].installed){COLOR_SET(VGA_COL_WARNING);printf("  '%s' already installed.\n",name);COLOR_RESET();return;}
    PKG_Install(name);
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  Installed: %s v%s\n",g_packages[idx].name,g_packages[idx].version);
    printf("  Run with: %s\n",g_packages[idx].name);
    COLOR_RESET();
}

static void cmd_uninstall(ShellCmd *cl)
{
    if(cl->argc<2){cprintln("  Usage: uninstall <pkg>",VGA_COL_WARNING);return;}
    const char *name=cl->argv[1];
    if(cmd_strcmp(name,"all")==0){
        int count=0;
        for(int i=0;i<g_package_count;i++) if(g_packages[i].installed){g_packages[i].installed=false;count++;}
        COLOR_SET(VGA_COL_SUCCESS);printf("  %d package(s) uninstalled.\n",count);COLOR_RESET();
        return;
    }
    int idx=PKG_Find(name);
    if(idx<0){COLOR_SET(VGA_COL_ERROR);printf("  '%s' not found.\n",name);COLOR_RESET();return;}
    if(!g_packages[idx].installed){COLOR_SET(VGA_COL_WARNING);printf("  '%s' not installed.\n",name);COLOR_RESET();return;}
    PKG_Uninstall(name);
    COLOR_SET(VGA_COL_SUCCESS);printf("  Uninstalled: %s\n",name);COLOR_RESET();
}

static void cmd_reboot(ShellCmd *cl)
{
    (void)cl;
    cprintln("  Rebooting OmniOS...",VGA_COL_WARNING);
    uint8_t tmp; do{tmp=i686_inb(0x64);}while(tmp&0x02);
    i686_outb(0x64,0xFE);
    __asm__ volatile("cli;hlt");
}

static void cmd_halt(ShellCmd *cl)
{
    (void)cl;
    cprintln("  OmniOS halted. Safe to power off.",VGA_COL_WARNING);
    __asm__ volatile("cli"); while(1)__asm__ volatile("hlt");
}

static void cmd_panic(ShellCmd *cl)
{
    (void)cl;
    VGA_SetColor(VGA_MAKE_COLOR(VGA_COLOR_WHITE,VGA_COLOR_RED));
    VGA_ClearScreen();
    puts("\n\n  *** KERNEL PANIC ***\n\n  Manual panic triggered.\n  System halted.\n\n  Reboot your machine.\n");
    __asm__ volatile("cli"); while(1)__asm__ volatile("hlt");
}

/* ============================================================
   Dispatch table
   ============================================================ */
typedef struct{const char *name;void(*fn)(ShellCmd*);}CmdEntry;

static const CmdEntry s_dispatch[]={
    {"help",cmd_help},{"?",cmd_help},
    {"clear",cmd_clear},{"cls",cmd_clear},
    {"echo",cmd_echo},
    {"version",cmd_version},{"ver",cmd_version},
    {"sysinfo",cmd_sysinfo},
    {"meminfo",cmd_meminfo},
    {"cpuinfo",cmd_cpuinfo},
    {"history",cmd_history},{"hist",cmd_history},
    {"color",cmd_color},{"colour",cmd_color},
    {"calc",cmd_calc},
    {"ascii",cmd_ascii},
    {"hex",cmd_hex},
    {"peek",cmd_peek},
    {"poke",cmd_poke},
    {"uptime",cmd_uptime},
    {"date",cmd_date},{"time",cmd_date},
    {"whoami",cmd_whoami},
    {"hostname",cmd_hostname},
    {"banner",cmd_banner},
    {"packages",cmd_packages},{"pkg",cmd_packages},
    {"install",cmd_install},
    {"uninstall",cmd_uninstall},{"remove",cmd_uninstall},
    {"reboot",cmd_reboot},
    {"halt",cmd_halt},{"shutdown",cmd_halt},
    {"panic",cmd_panic},
    {NULL,NULL}
};

void Commands_Dispatch(ShellCmd *cl)
{
    if(!cl||!cl->cmd[0]) return;

    for(int i=0;s_dispatch[i].name;i++){
        if(cmd_strcmp(cl->cmd,s_dispatch[i].name)==0){
            s_dispatch[i].fn(cl);
            return;
        }
    }

    if(PKG_Run(cl->cmd,cl)) return;

    int pkg_idx=PKG_Find(cl->cmd);
    if(pkg_idx>=0){
        COLOR_SET(VGA_COL_WARNING);
        printf("  Package '%s' not installed.\n",cl->cmd);
        COLOR_RESET();
        COLOR_SET(VGA_COL_INFO);
        printf("  Install with: install %s\n",cl->cmd);
        COLOR_RESET();
        return;
    }

    COLOR_SET(VGA_COL_ERROR);
    printf("  Unknown command: '%s'\n",cl->cmd);
    COLOR_RESET();
    cprintln("  Type 'help' to list commands.",VGA_COL_INFO);
}