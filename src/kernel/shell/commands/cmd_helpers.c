/* ============================================================
   OmniOS - Shared command helpers implementation
   ============================================================ */
#include "cmd_helpers.h"

void cprint(const char *s, uint8_t col)
{
    COLOR_SET(col); puts(s); COLOR_RESET();
}

void cprintln(const char *s, uint8_t col)
{
    COLOR_SET(col); puts(s); puts("\n"); COLOR_RESET();
}

void hr(char ch, int w, uint8_t col)
{
    COLOR_SET(col);
    for (int i = 0; i < w; i++) putc(ch);
    putc('\n');
    COLOR_RESET();
}

int cmd_strlen(const char *s)
{
    int n = 0; while (s[n]) n++; return n;
}

int cmd_strcmp(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int cmd_atoi(const char *s)
{
    int r = 0, neg = 0;
    if (*s == '-') { neg = 1; s++; }
    while (*s >= '0' && *s <= '9') r = r * 10 + (*s++ - '0');
    return neg ? -r : r;
}

void cmd_strcpy(char *d, const char *s)
{
    while (*s) *d++ = *s++;
    *d = '\0';
}

uintptr_t parse_addr(const char *s)
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

void pkg_delay(uint32_t ticks)
{
    uint32_t start = g_tick_count;
    while (g_tick_count - start < ticks) __asm__ volatile("hlt");
}

static uint32_t s_rand_seed = 0;
uint32_t pkg_rand(void)
{
    if (s_rand_seed == 0) s_rand_seed = g_tick_count;
    s_rand_seed = s_rand_seed * 1103515245 + 12345;
    return (s_rand_seed >> 16) & 0x7FFF;
}

void spk_on(uint32_t freq)
{
    uint32_t div = 1193180 / freq;
    i686_outb(0x43, 0xB6);
    i686_outb(0x42, (uint8_t)(div & 0xFF));
    i686_outb(0x42, (uint8_t)((div >> 8) & 0xFF));
    uint8_t tmp = i686_inb(0x61);
    i686_outb(0x61, tmp | 0x03);
}

void spk_off(void)
{
    uint8_t tmp = i686_inb(0x61);
    i686_outb(0x61, tmp & 0xFC);
}

void kv_row(const char *key, const char *val, int key_w)
{
    COLOR_SET(VGA_COL_INFO);
    puts("  "); puts(key);
    int pad = key_w - cmd_strlen(key);
    for (int i = 0; i < pad; i++) putc(' ');
    COLOR_RESET();
    puts(": "); puts(val); putc('\n');
}