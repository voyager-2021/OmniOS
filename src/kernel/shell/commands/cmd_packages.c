#include "cmd_helpers.h"

void cmd_packages(ShellCmd *cl)
{
    (void)cl;
    hr('=', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("   OmniOS Package Manager", VGA_COL_HEADER);
    hr('=', VGA_WIDTH, VGA_COL_HEADER);
    if (g_package_count == 0) { cprintln("  No packages.", VGA_COL_WARNING); return; }
    COLOR_SET(VGA_COL_INFO);
    printf("  %-16s %-8s %-10s %s\n", "PACKAGE", "VER", "STATUS", "DESCRIPTION");
    COLOR_RESET();
    hr('-', VGA_WIDTH, VGA_COL_NORMAL);
    for (int i = 0; i < g_package_count; i++) {
        Package *p = &g_packages[i];
        if (p->installed) {
            COLOR_SET(VGA_COL_SUCCESS);
            printf("  %-16s %-8s ", p->name, p->version);
            cprint("[installed]", VGA_COL_SUCCESS);
        } else {
            COLOR_SET(VGA_COL_NORMAL);
            printf("  %-16s %-8s ", p->name, p->version);
            cprint("[  ready  ]", VGA_COL_WARNING);
        }
        COLOR_SET(VGA_COL_NORMAL);
        printf(" %s\n", p->description);
    }
    hr('-', VGA_WIDTH, VGA_COL_NORMAL);
    cprintln("  install <name> | install all | uninstall <name>", VGA_COL_INFO);
}