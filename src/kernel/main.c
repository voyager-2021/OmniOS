/* ============================================================
   OmniOS v1.0.0 - Kernel Entry Point
   ============================================================ */
#include <stdint.h>
#include <stdbool.h>
#include "arch/i686/vga_text.h"
#include "arch/i686/gdt.h"
#include "arch/i686/idt.h"
#include "arch/i686/irq.h"
#include "arch/i686/isr.h"
#include "arch/i686/i8259.h"
#include "arch/i686/io.h"
#include "hal/hal.h"
#include "drivers/keyboard.h"
#include "drivers/ata.h"
#include "drivers/fat.h"
#include "shell/shell.h"
#include "shell/commands.h"
#include "stdio.h"
#include "debug.h"

#define OMNIOS_VERSION  "1.0.0"
#define OMNIOS_BUILD    "2025.001"

volatile uint32_t g_tick_count = 0;
ATA_Drive g_ata_drive;
bool      g_fs_mounted = false;

static void timer_handler(Registers *regs)
{
    (void)regs;
    g_tick_count++;
}

static void delay_ticks(uint32_t ticks)
{
    uint32_t start = g_tick_count;
    while (g_tick_count - start < ticks)
        __asm__ volatile("hlt");
}

static void anim_progress_bar(const char *label, int width, uint8_t col)
{
    VGA_SetColor(VGA_COL_INFO);
    puts("  "); puts(label); puts(" [");
    VGA_SetColor(col);
    for (int i = 0; i < width; i++) {
        putc('#');
        if (i % 2 == 0) delay_ticks(1);
    }
    VGA_SetColor(VGA_COL_INFO);
    puts("] ");
    VGA_SetColor(VGA_COL_SUCCESS);
    puts("OK\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

static void boot_status(const char *msg, bool ok)
{
    VGA_SetColor(ok ? VGA_COL_SUCCESS : VGA_COL_ERROR);
    puts(ok ? "  [  OK  ] " : "  [ FAIL ] ");
    VGA_SetColor(VGA_COL_NORMAL);
    puts(msg); puts("\n");
    delay_ticks(1);
}

static void startup_animation(void)
{
    VGA_ClearScreen();

    VGA_SetColor(VGA_COL_HEADER);
    static const char *logo[] = {
        "                                                                ",
        "     ::::::::  :::   ::: ::::    ::: :::::::::  ::::::::  :::::",
        "    :+:    :+: :+:  :+: :+:+:   :+:  :+:    :+::+:    :+::+:  ",
        "    +:+    +:+ +:+  +:+ :+:+:+  +:+  +:+    +:++:+    +:++:+  ",
        "    +#+    +:+ +#++:+++ +#+ +:+ +#+  +#+    +:++#+    +:++#++:",
        "    +#+    +#+ +#+  +#+ +#+  +#+#+#  +#+    +#++#+    +#+    +#+",
        "    #+#    #+# #+#  #+# #+#   #+#+#  #+#    #+##+#    #+#    #+#",
        "     ########  ###  ### ###    ####  #########  ########  :::::",
        "                                                                ",
    };
    for (int i = 0; i < 9; i++) {
        puts(logo[i]); puts("\n");
        delay_ticks(1);
    }
    VGA_SetColor(VGA_COL_NORMAL);
    puts("\n");

    VGA_SetColor(VGA_COL_INFO);
    printf("    OmniOS v%s  |  Build %s  |  x86 32-bit\n\n", OMNIOS_VERSION, OMNIOS_BUILD);
    VGA_SetColor(VGA_COL_NORMAL);

    VGA_DrawHLine('=', VGA_WIDTH, VGA_COL_HEADER);
    puts("\n");

    boot_status("Initialising VGA text mode 80x25",      true);
    boot_status("Loading GDT (code/data segments)",       true);
    boot_status("Loading IDT (256 interrupt gates)",       true);
    boot_status("Configuring PIC i8259 (IRQ remap)",      true);
    boot_status("Installing ISR/IRQ handlers",             true);
    boot_status("Hooking PIT timer (IRQ0, ~18.2 Hz)",    true);
    boot_status("Starting PS/2 keyboard driver (IRQ1)",  true);
    boot_status("Detecting ATA disk drive",               g_ata_drive.present);
    boot_status("Mounting FAT32 filesystem",              g_fs_mounted);

    puts("\n");
    anim_progress_bar("Loading kernel modules", 40, VGA_COL_SUCCESS);
    anim_progress_bar("Initialising shell    ", 40,
                      VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));

    puts("\n");
    VGA_DrawHLine('-', VGA_WIDTH, VGA_COL_INFO);
    VGA_SetColor(VGA_COL_SUCCESS);
    printf("  OmniOS v%s booted successfully!\n", OMNIOS_VERSION);
    VGA_SetColor(VGA_COL_INFO);
    puts("  Type 'help' for commands.  'ls' to list files.  'packages' for apps.\n");
    VGA_DrawHLine('-', VGA_WIDTH, VGA_COL_INFO);
    puts("\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

static uint32_t find_partition_offset(ATA_Drive *drive)
{
    static uint8_t mbr[512];
    if (!ATA_ReadSectors(drive, 0, 1, mbr))
        return 0;
    if (mbr[510] != 0x55 || mbr[511] != 0xAA)
        return 0;
    return *(uint32_t *)(mbr + 0x1BE + 0x08);
}

void kernel_main(void)
{
    HAL_Initialize();
    VGA_Initialize();

    i686_IRQ_RegisterHandler(0, timer_handler);
    KB_Initialize();

    __asm__ volatile("sti");

    ATA_Initialize(&g_ata_drive);

    if (g_ata_drive.present) {
        uint32_t part_offset = find_partition_offset(&g_ata_drive);
        if (part_offset > 0) {
            g_fs_mounted = KFAT_Initialize(&g_ata_drive, part_offset);
        }
    }

    startup_animation();
    Shell_Run();

    __asm__ volatile("cli");
    while (1) __asm__ volatile("hlt");
}