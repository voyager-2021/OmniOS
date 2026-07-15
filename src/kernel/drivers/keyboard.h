#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "../arch/i686/isr.h"

/* ============================================================
   OmniOS - PS/2 Keyboard Driver
   ============================================================ */

#define KB_DATA_PORT    0x60
#define KB_STATUS_PORT  0x64

#define KB_KEY_NONE     0x00
#define KB_KEY_UP       0x80
#define KB_KEY_DOWN     0x81
#define KB_KEY_LEFT     0x82
#define KB_KEY_RIGHT    0x83
#define KB_KEY_DELETE   0x84
#define KB_KEY_HOME     0x85
#define KB_KEY_END      0x86
#define KB_KEY_PGUP     0x87
#define KB_KEY_PGDN     0x88
#define KB_KEY_F1       0x89
#define KB_KEY_F2       0x8A
#define KB_KEY_F3       0x8B
#define KB_KEY_F4       0x8C
#define KB_KEY_F5       0x8D
#define KB_KEY_F6       0x8E
#define KB_KEY_F7       0x8F
#define KB_KEY_F8       0x90
#define KB_KEY_F9       0x91
#define KB_KEY_F10      0x92
#define KB_KEY_F11      0x93
#define KB_KEY_F12      0x94
#define KB_KEY_ESC      0x95
#define KB_KEY_INSERT   0x96

typedef struct {
    bool shift;
    bool ctrl;
    bool alt;
    bool caps_lock;
    bool num_lock;
    bool scroll_lock;
} KB_Modifiers;

void         KB_Initialize(void);
char         KB_GetChar(void);
char         KB_TryGetChar(void);
bool         KB_KeyAvailable(void);
KB_Modifiers KB_GetModifiers(void);
void         KB_Flush(void);