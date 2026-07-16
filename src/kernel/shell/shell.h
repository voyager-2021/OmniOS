#pragma once
#include <stdint.h>
#include <stdbool.h>

/* ============================================================
   OmniOS v1.0.0 - Shell Header
   ============================================================ */

#define OMNIOS_VERSION      "1.0.0"
#define OMNIOS_BUILD        "2025.001"

#define SHELL_PROMPT        "omni> "
#define SHELL_MAX_INPUT     256
#define SHELL_MAX_ARGS      32
#define SHELL_HISTORY_SIZE  32

/* Max packages */
#define PKG_MAX_PACKAGES    32
#define PKG_NAME_LEN        32
#define PKG_DESC_LEN        64
#define PKG_VER_LEN         16

typedef struct {
    char  raw[SHELL_MAX_INPUT];
    char  cmd[SHELL_MAX_INPUT];
    char *argv[SHELL_MAX_ARGS];
    int   argc;
    char  tokens[SHELL_MAX_INPUT];
} ShellCmd;

/* Package definition */
typedef struct {
    char name[PKG_NAME_LEN];
    char description[PKG_DESC_LEN];
    char version[PKG_VER_LEN];
    bool installed;
    void (*on_install)(void);
    void (*on_run)(ShellCmd *cl);
} Package;

/* ---- History API ---- */
extern int         s_hist_count;
const char        *history_get(int offset);

/* ---- Package API ---- */
extern Package     g_packages[];
extern int         g_package_count;

void PKG_Init(void);
int  PKG_Find(const char *name);
bool PKG_Install(const char *name);
bool PKG_Uninstall(const char *name);
bool PKG_IsInstalled(const char *name);
bool PKG_Run(const char *name, ShellCmd *cl);

/* ---- Shell API ---- */
void Shell_Run(void);
void Shell_ParseLine(const char *input, ShellCmd *out);
void Shell_PrintPrompt(void);
void Shell_ReadLine(char *buf, int maxlen);