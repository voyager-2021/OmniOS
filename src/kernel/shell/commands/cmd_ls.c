#include "cmd_helpers.h"
#include "../../drivers/fat.h"

extern bool g_fs_mounted;

void cmd_ls(ShellCmd *cl)
{
    if (!g_fs_mounted) { cprintln("  No filesystem mounted.", VGA_COL_ERROR); return; }
    const char *path = "/";
    if (cl->argc >= 2) path = cl->argv[1];
    KFAT_ListDir(path);
}