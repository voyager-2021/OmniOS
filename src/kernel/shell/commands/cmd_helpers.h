#pragma once
#include "../shell.h"
#include "../../stdio.h"
#include "../../arch/i686/vga_text.h"
#include "../../arch/i686/io.h"
#include "../../drivers/keyboard.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================
   OmniOS - Shared command helpers
   ============================================================ */

#define COLOR_SET(c)   VGA_SetColor(c)
#define COLOR_RESET()  VGA_SetColor(VGA_COL_NORMAL)

extern volatile uint32_t g_tick_count;

void      cprint(const char *s, uint8_t col);
void      cprintln(const char *s, uint8_t col);
void      hr(char ch, int w, uint8_t col);
int       cmd_strlen(const char *s);
int       cmd_strcmp(const char *a, const char *b);
int       cmd_atoi(const char *s);
void      cmd_strcpy(char *d, const char *s);
uintptr_t parse_addr(const char *s);
void      pkg_delay(uint32_t ticks);
uint32_t  pkg_rand(void);
void      spk_on(uint32_t freq);
void      spk_off(void);
void      kv_row(const char *key, const char *val, int key_w);