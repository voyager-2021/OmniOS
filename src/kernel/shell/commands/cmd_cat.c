#include "cmd_helpers.h"
#include "../../drivers/fat.h"

extern bool g_fs_mounted;

void cmd_cat(ShellCmd *cl)
{
    if (!g_fs_mounted) { cprintln("  No filesystem mounted.", VGA_COL_ERROR); return; }
    if (cl->argc < 2) { cprintln("  Usage: cat <file>", VGA_COL_WARNING); return; }
    KFAT_CatFile(cl->argv[1]);
}