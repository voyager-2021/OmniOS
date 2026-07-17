#include "cmd_helpers.h"
#include "../../drivers/ata.h"

extern ATA_Drive g_ata_drive;
extern bool      g_fs_mounted;

void cmd_disk(ShellCmd *cl)
{
    (void)cl;
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    cprintln("   Disk Information", VGA_COL_HEADER);
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
    if (g_ata_drive.present) {
        ATA_PrintInfo(&g_ata_drive);
        COLOR_SET(VGA_COL_INFO);
        printf("  FS Mounted: %s\n", g_fs_mounted ? "Yes (FAT32)" : "No");
        COLOR_RESET();
    } else {
        cprintln("  No ATA drive detected.", VGA_COL_WARNING);
    }
    hr('-', VGA_WIDTH, VGA_COL_HEADER);
}