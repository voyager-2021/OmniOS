/* ============================================================
   OmniOS - Kernel stdio
   Wires putc/puts/printf to the VGA text driver.
   Supports: %d %i %u %x %X %o %b %s %c %p %% with
             width, zero-padding, left-align (-), sign (+)
   ============================================================ */
#include "stdio.h"
#include "arch/i686/vga_text.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ---- Low-level output ---- */

void putc(char c)
{
    VGA_PutChar(c);
}

int putchar(int c)
{
    VGA_PutChar((char)c);
    return (unsigned char)c;
}

void puts(const char *s)
{
    if (!s) s = "(null)";
    while (*s) VGA_PutChar(*s++);
}

/* ---- Internal: write a single char into a buffer or to VGA ---- */
typedef struct {
    char  *buf;
    int    size;   /* 0 = unbounded (write to VGA) */
    int    pos;
} PrintCtx;

static void ctx_putc(PrintCtx *ctx, char c)
{
    if (!ctx->buf) {
        VGA_PutChar(c);
    } else {
        if (ctx->size == 0 || ctx->pos < ctx->size - 1) {
            ctx->buf[ctx->pos] = c;
        }
    }
    ctx->pos++;
}

static void ctx_puts(PrintCtx *ctx, const char *s)
{
    if (!s) s = "(null)";
    while (*s) ctx_putc(ctx, *s++);
}

/* ---- Number formatting ---- */
static void fmt_number(PrintCtx *ctx,
                        uint64_t  val,
                        int       base,
                        bool      upper,
                        bool      is_signed,
                        bool      negative,
                        int       width,
                        char      pad,
                        bool      left_align,
                        bool      show_sign,
                        bool      alt_form)
{
    static const char *lo = "0123456789abcdef";
    static const char *hi = "0123456789ABCDEF";
    const char *digits = upper ? hi : lo;

    char   tmp[66];
    int    len = 0;

    if (val == 0) {
        tmp[len++] = '0';
    } else {
        uint64_t v = val;
        while (v > 0) {
            tmp[len++] = digits[v % base];
            v /= base;
        }
    }

    /* Build prefix string */
    char prefix[4] = {0};
    int  pfx_len   = 0;
    if (negative)        prefix[pfx_len++] = '-';
    else if (show_sign)  prefix[pfx_len++] = '+';
    if (alt_form && base == 16 && val != 0) {
        prefix[pfx_len++] = '0';
        prefix[pfx_len++] = upper ? 'X' : 'x';
    } else if (alt_form && base == 8 && val != 0) {
        prefix[pfx_len++] = '0';
    }

    int total = len + pfx_len;
    int pad_n = (width > total) ? (width - total) : 0;

    if (!left_align && pad == '0') {
        /* prefix first, then zero padding */
        for (int i = 0; i < pfx_len; i++) ctx_putc(ctx, prefix[i]);
        for (int i = 0; i < pad_n; i++)   ctx_putc(ctx, '0');
    } else if (!left_align) {
        for (int i = 0; i < pad_n; i++)   ctx_putc(ctx, ' ');
        for (int i = 0; i < pfx_len; i++) ctx_putc(ctx, prefix[i]);
    } else {
        for (int i = 0; i < pfx_len; i++) ctx_putc(ctx, prefix[i]);
    }

    /* Digits (they are reversed in tmp) */
    for (int i = len - 1; i >= 0; i--) ctx_putc(ctx, tmp[i]);

    if (left_align)
        for (int i = 0; i < pad_n; i++) ctx_putc(ctx, ' ');
}

/* ---- Core formatter ---- */
static int do_printf(PrintCtx *ctx, const char *fmt, va_list ap)
{
    while (*fmt) {
        if (*fmt != '%') { ctx_putc(ctx, *fmt++); continue; }
        fmt++; /* skip '%' */

        /* Flags */
        bool left_align = false;
        bool show_sign  = false;
        bool alt_form   = false;
        bool done_flags = false;
        while (!done_flags) {
            switch (*fmt) {
                case '-': left_align = true; fmt++; break;
                case '+': show_sign  = true; fmt++; break;
                case '#': alt_form   = true; fmt++; break;
                case ' ': fmt++;             break;
                default:  done_flags = true; break;
            }
        }

        /* Pad char */
        char pad = ' ';
        if (*fmt == '0' && !left_align) { pad = '0'; fmt++; }

        /* Width */
        int width = 0;
        if (*fmt == '*') {
            width = va_arg(ap, int);
            if (width < 0) { left_align = true; width = -width; }
            fmt++;
        } else {
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }
        }

        /* Precision (ignored for now, just consume) */
        if (*fmt == '.') {
            fmt++;
            if (*fmt == '*') { va_arg(ap, int); fmt++; }
            else while (*fmt >= '0' && *fmt <= '9') fmt++;
        }

        /* Length modifier */
        bool is_long  = false;
        bool is_llong = false;
        if (*fmt == 'l') {
            fmt++;
            if (*fmt == 'l') { is_llong = true; fmt++; }
            else              { is_long  = true; }
        } else if (*fmt == 'h') {
            fmt++; /* treat as int */
            if (*fmt == 'h') fmt++;
        }

        /* Specifier */
        char spec = *fmt++;
        switch (spec) {
            case 'd': case 'i': {
                int64_t v = is_llong ? va_arg(ap, long long)
                          : is_long  ? va_arg(ap, long)
                                     : va_arg(ap, int);
                bool neg = (v < 0);
                uint64_t uv = neg ? (uint64_t)(-(v + 1)) + 1 : (uint64_t)v;
                fmt_number(ctx, uv, 10, false, true, neg,
                           width, pad, left_align, show_sign, false);
                break;
            }
            case 'u': {
                uint64_t v = is_llong ? va_arg(ap, unsigned long long)
                           : is_long  ? va_arg(ap, unsigned long)
                                      : va_arg(ap, unsigned int);
                fmt_number(ctx, v, 10, false, false, false,
                           width, pad, left_align, show_sign, false);
                break;
            }
            case 'x': {
                uint64_t v = is_llong ? va_arg(ap, unsigned long long)
                           : is_long  ? va_arg(ap, unsigned long)
                                      : va_arg(ap, unsigned int);
                fmt_number(ctx, v, 16, false, false, false,
                           width, pad, left_align, false, alt_form);
                break;
            }
            case 'X': {
                uint64_t v = is_llong ? va_arg(ap, unsigned long long)
                           : is_long  ? va_arg(ap, unsigned long)
                                      : va_arg(ap, unsigned int);
                fmt_number(ctx, v, 16, true, false, false,
                           width, pad, left_align, false, alt_form);
                break;
            }
            case 'o': {
                uint64_t v = is_llong ? va_arg(ap, unsigned long long)
                           : is_long  ? va_arg(ap, unsigned long)
                                      : va_arg(ap, unsigned int);
                fmt_number(ctx, v, 8, false, false, false,
                           width, pad, left_align, false, alt_form);
                break;
            }
            case 'b': {
                /* binary - OmniOS extension */
                uint64_t v = is_llong ? va_arg(ap, unsigned long long)
                           : is_long  ? va_arg(ap, unsigned long)
                                      : va_arg(ap, unsigned int);
                fmt_number(ctx, v, 2, false, false, false,
                           width, pad, left_align, false, false);
                break;
            }
            case 'p': {
                uintptr_t v = (uintptr_t)va_arg(ap, void *);
                ctx_puts(ctx, "0x");
                fmt_number(ctx, (uint64_t)v, 16, false, false, false,
                           8, '0', false, false, false);
                break;
            }
            case 's': {
                const char *s = va_arg(ap, const char *);
                if (!s) s = "(null)";
                int slen = 0;
                const char *t = s;
                while (*t++) slen++;
                int pad_n = (width > slen) ? width - slen : 0;
                if (!left_align)
                    for (int i = 0; i < pad_n; i++) ctx_putc(ctx, ' ');
                ctx_puts(ctx, s);
                if (left_align)
                    for (int i = 0; i < pad_n; i++) ctx_putc(ctx, ' ');
                break;
            }
            case 'c':
                ctx_putc(ctx, (char)va_arg(ap, int));
                break;
            case '%':
                ctx_putc(ctx, '%');
                break;
            case 0:
                goto done;
            default:
                ctx_putc(ctx, '%');
                ctx_putc(ctx, spec);
                break;
        }
    }
done:
    if (ctx->buf && (ctx->size == 0 || ctx->pos < ctx->size))
        ctx->buf[ctx->pos] = '\0';
    return ctx->pos;
}

/* ---- Public API ---- */

int vprintf(const char *fmt, va_list args)
{
    PrintCtx ctx = { .buf = NULL, .size = 0, .pos = 0 };
    return do_printf(&ctx, fmt, args);
}

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vprintf(fmt, ap);
    va_end(ap);
    return r;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
    PrintCtx ctx = { .buf = buf, .size = 0, .pos = 0 };
    return do_printf(&ctx, fmt, args);
}

int sprintf(char *buf, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vsprintf(buf, fmt, ap);
    va_end(ap);
    return r;
}

int snprintf(char *buf, int size, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    PrintCtx ctx = { .buf = buf, .size = size, .pos = 0 };
    int r = do_printf(&ctx, fmt, ap);
    va_end(ap);
    return r;
}