#pragma once
#include <stdint.h>
#include <stdbool.h>

/* ============================================================
   OmniOS - Shell Header
   ============================================================ */

#define SHELL_PROMPT        "omni> "
#define SHELL_MAX_INPUT     256
#define SHELL_MAX_ARGS      32
#define SHELL_HISTORY_SIZE  32

typedef struct {
    char  raw[SHELL_MAX_INPUT];
    char  cmd[SHELL_MAX_INPUT];
    char *argv[SHELL_MAX_ARGS];
    int   argc;
    char  tokens[SHELL_MAX_INPUT];
} ShellCmd;

void Shell_Run(void);
void Shell_ParseLine(const char *input, ShellCmd *out);
void Shell_PrintPrompt(void);
void Shell_ReadLine(char *buf, int maxlen);