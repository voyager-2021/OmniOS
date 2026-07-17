/* ============================================================
   OmniOS v1.0.0 - Command dispatch table
   ============================================================ */
#include "commands.h"
#include "shell.h"
#include "commands/cmd_helpers.h"

typedef struct {
    const char *name;
    void (*fn)(ShellCmd *);
} CmdEntry;

static const CmdEntry s_dispatch[] = {
    {"help",      cmd_help},
    {"?",         cmd_help},
    {"clear",     cmd_clear},
    {"cls",       cmd_clear},
    {"echo",      cmd_echo},
    {"version",   cmd_version},
    {"ver",       cmd_version},
    {"sysinfo",   cmd_sysinfo},
    {"meminfo",   cmd_meminfo},
    {"cpuinfo",   cmd_cpuinfo},
    {"history",   cmd_history},
    {"hist",      cmd_history},
    {"color",     cmd_color},
    {"colour",    cmd_color},
    {"calc",      cmd_calc},
    {"ascii",     cmd_ascii},
    {"hex",       cmd_hex},
    {"peek",      cmd_peek},
    {"poke",      cmd_poke},
    {"uptime",    cmd_uptime},
    {"date",      cmd_date},
    {"time",      cmd_date},
    {"whoami",    cmd_whoami},
    {"hostname",  cmd_hostname},
    {"ls",        cmd_ls},
    {"dir",       cmd_ls},
    {"cat",       cmd_cat},
    {"type",      cmd_cat},
    {"disk",      cmd_disk},
    {"banner",    cmd_banner},
    {"packages",  cmd_packages},
    {"pkg",       cmd_packages},
    {"install",   cmd_install},
    {"uninstall", cmd_uninstall},
    {"remove",    cmd_uninstall},
    {"reboot",    cmd_reboot},
    {"halt",      cmd_halt},
    {"shutdown",  cmd_halt},
    {"panic",     cmd_panic},
    {NULL,        NULL}
};

void Commands_Dispatch(ShellCmd *cl)
{
    if (!cl || !cl->cmd[0]) return;

    for (int i = 0; s_dispatch[i].name; i++) {
        if (cmd_strcmp(cl->cmd, s_dispatch[i].name) == 0) {
            s_dispatch[i].fn(cl);
            return;
        }
    }

    if (PKG_Run(cl->cmd, cl)) return;

    int pkg_idx = PKG_Find(cl->cmd);
    if (pkg_idx >= 0) {
        COLOR_SET(VGA_COL_WARNING);
        printf("  Package '%s' not installed.\n", cl->cmd);
        COLOR_RESET();
        COLOR_SET(VGA_COL_INFO);
        printf("  Install with: install %s\n", cl->cmd);
        COLOR_RESET();
        return;
    }

    COLOR_SET(VGA_COL_ERROR);
    printf("  Unknown command: '%s'\n", cl->cmd);
    COLOR_RESET();
    cprintln("  Type 'help' to list commands.", VGA_COL_INFO);
}