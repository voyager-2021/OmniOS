#pragma once
#include <stdint.h>
#include <stdbool.h>

/* ============================================================
   OmniOS - PS/2 Keyboard Driver
   ============================================================ */

#define KB_DATA_PORT    0x60
#define KB_STATUS_PORT  0x64

/* Special key codes returned by KB_GetChar */
#define KB_KEY_NONE     0x00
#define KB_KEY_UP       0x01
#define KB_KEY_DOWN     0x02
#define KB_KEY_LEFT     0x03
#define KB_KEY_RIGHT    0x04
#define KB_KEY_DELETE   0x7F
#define KB_KEY_HOME     0x05
#define KB_KEY_END      0x06
#define KB_KEY_PGUP     0x0B
#define KB_KEY_PGDN     0x0C
#define KB_KEY_F1       0x10
#define KB_KEY_F2       0x11
#define KB_KEY_F3       0x12
#define KB_KEY_F4       0x13
#define KB_KEY_F5       0x14
#define KB_KEY_F6       0x15
#define KB_KEY_F7       0x16
#define KB_KEY_F8       0x17
#define KB_KEY_F9       0x18
#define KB_KEY_F10      0x19
#define KB_KEY_F11      0x1A
#define KB_KEY_F12      0x1B
#define KB_KEY_ESC      0x1B

typedef struct {
    bool shift;
    bool ctrl;
    bool alt;
    bool caps_lock;
    bool num_lock;
    bool scroll_lock;
} KB_Modifiers;

/* Initialise keyboard (register IRQ1 handler) */
void KB_Initialize(void);

/* Blocking read - returns the next character or special key code */
char KB_GetChar(void);

/* Non-blocking check - returns KB_KEY_NONE if no key waiting */
char KB_TryGetChar(void);

/* Check if a key is available */
bool KB_KeyAvailable(void);

/* Get current modifier state */
KB_Modifiers KB_GetModifiers(void);

/* Flush the keyboard buffer */
void KB_Flush(void);