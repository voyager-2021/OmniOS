/* ============================================================
   OmniOS - PS/2 Keyboard Driver
   Handles: US QWERTY, shift, caps-lock, ctrl, alt,
            arrow keys (extended), F1-F12, Delete, Home, End,
            Page Up/Down via a 64-char ring buffer.
   ============================================================ */
#include "keyboard.h"
#include "../arch/i686/io.h"
#include "../arch/i686/irq.h"
#include <stdint.h>
#include <stdbool.h>

/* ---- US QWERTY scancode -> ASCII (set 1) ---- */
static const char sc_normal[] = {
/*00*/  0,    KB_KEY_ESC, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
/*0F*/  '\t', 'q','w','e','r','t','y','u','i','o','p','[',']','\n',
/*1D*/  0,    /* L-ctrl */
/*1E*/  'a','s','d','f','g','h','j','k','l',';','\'','`',
/*2A*/  0,    /* L-shift */
/*2B*/  '\\','z','x','c','v','b','n','m',',','.','/',
/*36*/  0,    /* R-shift */
/*37*/  '*',
/*38*/  0,    /* L-alt */
/*39*/  ' ',
/*3A*/  0,    /* Caps Lock */
/*3B*/  KB_KEY_F1, KB_KEY_F2, KB_KEY_F3, KB_KEY_F4,
/*3F*/  KB_KEY_F5, KB_KEY_F6, KB_KEY_F7, KB_KEY_F8,
/*43*/  KB_KEY_F9, KB_KEY_F10,
/*45*/  0,    /* Num Lock */
/*46*/  0,    /* Scroll Lock */
/*47*/  KB_KEY_HOME,
/*48*/  KB_KEY_UP,
/*49*/  KB_KEY_PGUP,
/*4A*/  '-',
/*4B*/  KB_KEY_LEFT,
/*4C*/  '5',
/*4D*/  KB_KEY_RIGHT,
/*4E*/  '+',
/*4F*/  KB_KEY_END,
/*50*/  KB_KEY_DOWN,
/*51*/  KB_KEY_PGDN,
/*52*/  0,   /* Insert */
/*53*/  KB_KEY_DELETE,
};

static const char sc_shift[] = {
/*00*/  0,    KB_KEY_ESC, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
/*0F*/  '\t', 'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
/*1D*/  0,
/*1E*/  'A','S','D','F','G','H','J','K','L',':','"','~',
/*2A*/  0,
/*2B*/  '|','Z','X','C','V','B','N','M','<','>','?',
/*36*/  0,
/*37*/  '*',
/*38*/  0,
/*39*/  ' ',
};

/* ---- Ring buffer ---- */
#define KB_BUF_SIZE 64

static volatile char   s_buf[KB_BUF_SIZE];
static volatile int    s_head = 0;
static volatile int    s_tail = 0;

/* ---- Modifier state ---- */
static volatile bool   s_shift      = false;
static volatile bool   s_ctrl       = false;
static volatile bool   s_alt        = false;
static volatile bool   s_caps_lock  = false;
static volatile bool   s_num_lock   = false;
static volatile bool   s_scroll_lock = false;
static volatile bool   s_extended   = false; /* 0xE0 prefix seen */

static void buf_push(char c)
{
    int next = (s_head + 1) % KB_BUF_SIZE;
    if (next != s_tail) { /* not full */
        s_buf[s_head] = c;
        s_head = next;
    }
}

static char buf_pop(void)
{
    if (s_tail == s_head) return KB_KEY_NONE;
    char c  = s_buf[s_tail];
    s_tail  = (s_tail + 1) % KB_BUF_SIZE;
    return c;
}

static bool buf_empty(void)
{
    return s_head == s_tail;
}

/* ---- IRQ1 handler ---- */
static void keyboard_handler(Registers *regs)
{
    (void)regs;
    uint8_t sc = i686_inb(KB_DATA_PORT);

    /* Extended key prefix */
    if (sc == 0xE0) { s_extended = true; return; }

    bool released = (sc & 0x80) != 0;
    sc &= 0x7F;

    if (s_extended) {
        s_extended = false;
        if (!released) {
            switch (sc) {
                case 0x48: buf_push(KB_KEY_UP);    return;
                case 0x50: buf_push(KB_KEY_DOWN);  return;
                case 0x4B: buf_push(KB_KEY_LEFT);  return;
                case 0x4D: buf_push(KB_KEY_RIGHT); return;
                case 0x47: buf_push(KB_KEY_HOME);  return;
                case 0x4F: buf_push(KB_KEY_END);   return;
                case 0x49: buf_push(KB_KEY_PGUP);  return;
                case 0x51: buf_push(KB_KEY_PGDN);  return;
                case 0x53: buf_push(KB_KEY_DELETE);return;
                case 0x1D: s_ctrl = true;           return;
                case 0x38: s_alt  = true;           return;
            }
        } else {
            switch (sc) {
                case 0x1D: s_ctrl = false; return;
                case 0x38: s_alt  = false; return;
            }
        }
        return;
    }

    /* Modifier keys */
    switch (sc) {
        case 0x2A: case 0x36: s_shift = !released;        return;
        case 0x1D:             s_ctrl  = !released;        return;
        case 0x38:             s_alt   = !released;        return;
        case 0x3A: if (!released) s_caps_lock   = !s_caps_lock;   return;
        case 0x45: if (!released) s_num_lock    = !s_num_lock;    return;
        case 0x46: if (!released) s_scroll_lock = !s_scroll_lock; return;
    }

    if (released) return;

    /* Translate scancode */
    char c = 0;
    if (sc < sizeof(sc_normal)) {
        if (s_shift) {
            c = (sc < (int)sizeof(sc_shift)) ? sc_shift[sc] : sc_normal[sc];
        } else {
            c = sc_normal[sc];
        }
    }

    if (!c) return;

    /* Caps lock flips alpha */
    if (s_caps_lock && c >= 'a' && c <= 'z') c -= 32;
    if (s_caps_lock && c >= 'A' && c <= 'Z') c += 32;

    /* Ctrl combinations */
    if (s_ctrl && c >= 'a' && c <= 'z') c -= 96; /* ctrl+a = 0x01 */

    buf_push(c);
}

/* ---- Public API ---- */

void KB_Initialize(void)
{
    s_head = s_tail = 0;
    s_shift = s_ctrl = s_alt = false;
    s_caps_lock = s_num_lock = s_scroll_lock = false;
    s_extended = false;
    IRQ_RegisterHandler(1, keyboard_handler);
}

char KB_GetChar(void)
{
    while (buf_empty()) { __asm__ volatile("hlt"); }
    return buf_pop();
}

char KB_TryGetChar(void)
{
    return buf_pop();
}

bool KB_KeyAvailable(void)
{
    return !buf_empty();
}

KB_Modifiers KB_GetModifiers(void)
{
    KB_Modifiers m;
    m.shift       = s_shift;
    m.ctrl        = s_ctrl;
    m.alt         = s_alt;
    m.caps_lock   = s_caps_lock;
    m.num_lock    = s_num_lock;
    m.scroll_lock = s_scroll_lock;
    return m;
}

void KB_Flush(void)
{
    s_head = s_tail = 0;
}