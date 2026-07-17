#include "cmd_helpers.h"

void cmd_uninstall(ShellCmd *cl)
{
    if (cl->argc < 2) { cprintln("  Usage: uninstall <pkg>", VGA_COL_WARNING); return; }
    const char *name = cl->argv[1];
    if (cmd_strcmp(name, "all") == 0) {
        int count = 0;
        for (int i = 0; i < g_package_count; i++)
            if (g_packages[i].installed) { g_packages[i].installed = false; count++; }
        COLOR_SET(VGA_COL_SUCCESS); printf("  %d package(s) uninstalled.\n", count); COLOR_RESET();
        return;
    }
    int idx = PKG_Find(name);
    if (idx < 0) { COLOR_SET(VGA_COL_ERROR); printf("  '%s' not found.\n", name); COLOR_RESET(); return; }
    if (!g_packages[idx].installed) { COLOR_SET(VGA_COL_WARNING); printf("  '%s' not installed.\n", name); COLOR_RESET(); return; }
    PKG_Uninstall(name);
    COLOR_SET(VGA_COL_SUCCESS); printf("  Uninstalled: %s\n", name); COLOR_RESET();
}