/* ============================================================
   OmniOS - Shell Core
   Provides: prompt, line editing (backspace, DEL, Home, End,
             left/right cursor), command history (up/down),
             tab-completion stub.
   ============================================================ */
#include "shell.h"
#include "commands.h"
#include "../stdio.h"
#include "../drivers/keyboard.h"
#include "../arch/i686/vga_text.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ---- String helpers ---- */
static int sh_strlen(const char *s)
{
    int n = 0; while (s[n]) n++; return n;
}

static void sh_strcpy(char *d, const char *s)
{
    while (*s) *d++ = *s++; *d = '\0';
}

static void sh_memmove(char *dst, const char *src, int n)
{
    if (dst < src) { for (int i = 0; i < n; i++) dst[i] = src[i]; }
    else           { for (int i = n-1; i >= 0; i--) dst[i] = src[i]; }
}

/* ---- History ---- */
static char  s_history[SHELL_HISTORY_SIZE][SHELL_MAX_INPUT];
static int   s_hist_count = 0;
static int   s_hist_head  = 0; /* next write slot (circular) */

static void history_push(const char *line)
{
    if (!line || !line[0]) return;
    /* Don't duplicate the last entry */
    if (s_hist_count > 0) {
        int last = (s_hist_head - 1 + SHELL_HISTORY_SIZE) % SHELL_HISTORY_SIZE;
        bool same = true;
        const char *a = s_history[last], *b = line;
        while (*a && *b && *a == *b) { a++; b++; }
        same = (*a == '\0' && *b == '\0');
        if (same) return;
    }
    sh_strcpy(s_history[s_hist_head], line);
    s_hist_head = (s_hist_head + 1) % SHELL_HISTORY_SIZE;
    if (s_hist_count < SHELL_HISTORY_SIZE) s_hist_count++;
}

static const char *history_get(int offset) /* offset 1 = most recent */
{
    if (offset < 1 || offset > s_hist_count) return NULL;
    int idx = (s_hist_head - offset + SHELL_HISTORY_SIZE * 2) % SHELL_HISTORY_SIZE;
    return s_history[idx];
}

/* ---- Line editor state ---- */
typedef struct {
    char buf[SHELL_MAX_INPUT];  /* current input buffer */
    int  len;                   /* chars in buf         */
    int  pos;                   /* cursor position      */
    int  hist_offset;           /* 0 = live buffer      */
    char saved[SHELL_MAX_INPUT];/* live buffer saved when browsing history */
} LineState;

static void line_clear_display(LineState *ls)
{
    /* Move cursor to start of prompt end, then erase to EOL using spaces */
    /* We track position by reprinting */
    int prompt_len = sh_strlen(SHELL_PROMPT);
    (void)prompt_len;

    /* Go back to right after the prompt */
    for (int i = 0; i < ls->pos; i++) VGA_PutChar('\b');
    /* Erase everything after prompt */
    for (int i = 0; i < ls->len; i++) VGA_PutChar(' ');
    for (int i = 0; i < ls->len; i++) VGA_PutChar('\b');
}

static void line_reprint(LineState *ls)
{
    VGA_SetColor(VGA_COL_INPUT);
    for (int i = 0; i < ls->len; i++) VGA_PutChar(ls->buf[i]);
    /* Move cursor back to position */
    for (int i = ls->len; i > ls->pos; i--) VGA_PutChar('\b');
}

static void line_insert(LineState *ls, char c)
{
    if (ls->len >= SHELL_MAX_INPUT - 1) return;
    /* Shift right */
    sh_memmove(ls->buf + ls->pos + 1, ls->buf + ls->pos, ls->len - ls->pos);
    ls->buf[ls->pos] = c;
    ls->len++;
    ls->buf[ls->len] = '\0';
    /* Redisplay from cursor */
    VGA_SetColor(VGA_COL_INPUT);
    for (int i = ls->pos; i < ls->len; i++) VGA_PutChar(ls->buf[i]);
    /* Move cursor back to after inserted char */
    for (int i = ls->len - 1; i > ls->pos; i--) VGA_PutChar('\b');
    ls->pos++;
}

static void line_backspace(LineState *ls)
{
    if (ls->pos == 0) return;
    sh_memmove(ls->buf + ls->pos - 1, ls->buf + ls->pos, ls->len - ls->pos);
    ls->len--;
    ls->pos--;
    ls->buf[ls->len] = '\0';
    VGA_PutChar('\b');
    VGA_SetColor(VGA_COL_INPUT);
    for (int i = ls->pos; i < ls->len; i++) VGA_PutChar(ls->buf[i]);
    VGA_PutChar(' ');
    for (int i = ls->len; i >= ls->pos; i--) VGA_PutChar('\b');
}

static void line_delete(LineState *ls)
{
    if (ls->pos >= ls->len) return;
    sh_memmove(ls->buf + ls->pos, ls->buf + ls->pos + 1, ls->len - ls->pos - 1);
    ls->len--;
    ls->buf[ls->len] = '\0';
    VGA_SetColor(VGA_COL_INPUT);
    for (int i = ls->pos; i < ls->len; i++) VGA_PutChar(ls->buf[i]);
    VGA_PutChar(' ');
    for (int i = ls->len; i >= ls->pos; i--) VGA_PutChar('\b');
}

static void line_load_history(LineState *ls, int offset)
{
    if (offset == 0) {
        /* Restore live buffer */
        line_clear_display(ls);
        sh_strcpy(ls->buf, ls->saved);
        ls->len = sh_strlen(ls->buf);
        ls->pos = ls->len;
        line_reprint(ls);
        ls->hist_offset = 0;
        return;
    }
    const char *h = history_get(offset);
    if (!h) return;
    if (ls->hist_offset == 0) sh_strcpy(ls->saved, ls->buf); /* save live */
    line_clear_display(ls);
    sh_strcpy(ls->buf, h);
    ls->len = sh_strlen(ls->buf);
    ls->pos = ls->len;
    ls->hist_offset = offset;
    line_reprint(ls);
}

/* ---- Public: Shell_ReadLine ---- */
void Shell_ReadLine(char *outbuf, int maxlen)
{
    LineState ls;
    ls.len = 0;
    ls.pos = 0;
    ls.hist_offset = 0;
    ls.buf[0] = '\0';
    ls.saved[0] = '\0';

    VGA_SetColor(VGA_COL_INPUT);

    while (true) {
        char c = KB_GetChar();

        switch ((unsigned char)c) {
            case '\n': case '\r':
                ls.buf[ls.len] = '\0';
                puts("\n");
                /* copy to output */
                int copy_len = ls.len < maxlen - 1 ? ls.len : maxlen - 1;
                for (int i = 0; i < copy_len; i++) outbuf[i] = ls.buf[i];
                outbuf[copy_len] = '\0';
                return;

            case '\b':      /* Backspace */
                line_backspace(&ls);
                break;

            case KB_KEY_DELETE:
                line_delete(&ls);
                break;

            case KB_KEY_LEFT:
                if (ls.pos > 0) { VGA_PutChar('\b'); ls.pos--; }
                break;

            case KB_KEY_RIGHT:
                if (ls.pos < ls.len) {
                    VGA_PutChar(ls.buf[ls.pos]);
                    ls.pos++;
                }
                break;

            case KB_KEY_HOME:
                while (ls.pos > 0) { VGA_PutChar('\b'); ls.pos--; }
                break;

            case KB_KEY_END:
                while (ls.pos < ls.len) {
                    VGA_PutChar(ls.buf[ls.pos]);
                    ls.pos++;
                }
                break;

            case KB_KEY_UP:
                if (ls.hist_offset < s_hist_count)
                    line_load_history(&ls, ls.hist_offset + 1);
                break;

            case KB_KEY_DOWN:
                if (ls.hist_offset > 0)
                    line_load_history(&ls, ls.hist_offset - 1);
                break;

            default:
                if ((unsigned char)c >= 0x20 && (unsigned char)c < 0x7F)
                    line_insert(&ls, c);
                break;
        }
    }
}

/* ---- Public: Shell_ParseLine ---- */
void Shell_ParseLine(const char *input, ShellCmd *cl)
{
    cl->argc = 0;
    cl->cmd[0] = '\0';

    sh_strcpy(cl->raw, input);
    sh_strcpy(cl->tokens, input);

    char *p = cl->tokens;
    while (*p == ' ') p++;

    while (*p && cl->argc < SHELL_MAX_ARGS) {
        cl->argv[cl->argc++] = p;
        while (*p && *p != ' ') p++;
        if (*p) { *p = '\0'; p++; }
        while (*p == ' ') p++;
    }

    if (cl->argc > 0) sh_strcpy(cl->cmd, cl->argv[0]);
}

/* ---- Public: Shell_PrintPrompt ---- */
void Shell_PrintPrompt(void)
{
    VGA_SetColor(VGA_COL_PROMPT);
    puts(SHELL_PROMPT);
    VGA_SetColor(VGA_COL_INPUT);
}

/* ---- Public: Shell_Run ---- */
void Shell_Run(void)
{
    static char input[SHELL_MAX_INPUT];
    ShellCmd cl;

    while (true) {
        Shell_PrintPrompt();
        Shell_ReadLine(input, SHELL_MAX_INPUT);

        if (!input[0]) continue;

        history_push(input);
        Shell_ParseLine(input, &cl);
        Commands_Dispatch(&cl);
    }
}