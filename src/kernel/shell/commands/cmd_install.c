#include "cmd_helpers.h"

void cmd_install(ShellCmd *cl)
{
    if (cl->argc < 2) { cprintln("  Usage: install <pkg>", VGA_COL_WARNING); return; }
    const char *name = cl->argv[1];
    if (cmd_strcmp(name, "all") == 0) {
        int count = 0;
        for (int i = 0; i < g_package_count; i++) {
            if (!g_packages[i].installed) {
                g_packages[i].installed = true;
                COLOR_SET(VGA_COL_SUCCESS);
                printf("  Installed: %s v%s\n", g_packages[i].name, g_packages[i].version);
                COLOR_RESET();
                count++;
            }
        }
        if (count == 0) cprintln("  All already installed.", VGA_COL_INFO);
        else { COLOR_SET(VGA_COL_SUCCESS); printf("  %d package(s) installed.\n", count); COLOR_RESET(); }
        return;
    }
    int idx = PKG_Find(name);
    if (idx < 0) { COLOR_SET(VGA_COL_ERROR); printf("  Package '%s' not found.\n", name); COLOR_RESET(); return; }
    if (g_packages[idx].installed) { COLOR_SET(VGA_COL_WARNING); printf("  '%s' already installed.\n", name); COLOR_RESET(); return; }
    PKG_Install(name);
    COLOR_SET(VGA_COL_SUCCESS);
    printf("  Installed: %s v%s\n", g_packages[idx].name, g_packages[idx].version);
    printf("  Run with: %s\n", g_packages[idx].name);
    COLOR_RESET();
}