#include "cmd_helpers.h"

void cmd_calc(ShellCmd *cl)
{
    if (cl->argc < 2) { cprintln("  Usage: calc <expr>", VGA_COL_WARNING); return; }
    static char expr[128];
    int ep = 0;
    for (int i = 1; i < cl->argc && ep < 126; i++)
        for (int j = 0; cl->argv[i][j] && ep < 126; j++)
            expr[ep++] = cl->argv[i][j];
    expr[ep] = '\0';

    char *p = expr;
    bool neg_a = 0;
    if (*p == '-') { neg_a = 1; p++; } else if (*p == '+') p++;
    int64_t a = 0;
    while (*p >= '0' && *p <= '9') a = a * 10 + (*p++ - '0');
    if (neg_a) a = -a;

    char op = *p++;
    if (!op) { COLOR_SET(VGA_COL_SUCCESS); printf("  = %d\n", (int)a); COLOR_RESET(); return; }

    bool neg_b = 0;
    if (*p == '-') { neg_b = 1; p++; } else if (*p == '+') p++;
    int64_t b = 0;
    while (*p >= '0' && *p <= '9') b = b * 10 + (*p++ - '0');
    if (neg_b) b = -b;

    int64_t result = 0;
    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': case 'x': result = a * b; break;
        case '/': if (b == 0) { cprintln("  Div by zero", VGA_COL_ERROR); return; } result = a / b; break;
        case '%': if (b == 0) { cprintln("  Mod by zero", VGA_COL_ERROR); return; } result = a % b; break;
        default: COLOR_SET(VGA_COL_ERROR); printf("  Unknown op '%c'\n", op); COLOR_RESET(); return;
    }
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  %d %c %d = %d\n", (int)a, op, (int)b, (int)result);
    COLOR_RESET();
}