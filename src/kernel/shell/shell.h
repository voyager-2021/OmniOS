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
    char  raw[SHELL_MAX_INPUT];     /* original input line        */
    char  cmd[SHELL_MAX_INPUT];     /* argv[0]                    */
    char *argv[SHELL_MAX_ARGS];     /* pointers into tokens[]     */
    int   argc;
    char  tokens[SHELL_MAX_INPUT];  /* null-separated token store */
} ShellCmd;

/* Initialise and start the shell (never returns) */
void Shell_Run(void);

/* Parse a raw input string into a ShellCmd */
void Shell_ParseLine(const char *input, ShellCmd *out);

/* Print the prompt */
void Shell_PrintPrompt(void);

/* Read a line of input (handles backspace, history nav) */
void Shell_ReadLine(char *buf, int maxlen);