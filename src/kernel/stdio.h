#pragma once
#include <stdint.h>
#include <stdarg.h>

/* ============================================================
   OmniOS - Kernel stdio
   ============================================================ */

void  putc(char c);
int   putchar(int c);
void  puts(const char *s);
int   printf(const char *fmt, ...);
int   vprintf(const char *fmt, va_list args);
int   sprintf(char *buf, const char *fmt, ...);
int   vsprintf(char *buf, const char *fmt, va_list args);
int   snprintf(char *buf, int size, const char *fmt, ...);