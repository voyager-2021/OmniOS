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
#include "shell/shell.h"
#include "shell/commands.h"
#include "stdio.h"
#include "debug.h"

#define OMNIOS_VERSION  "1.0.0"
#define OMNIOS_BUILD    "2025.001"

/* ---- Global tick counter ---- */
volatile uint32_t g_tick_count = 0;

static void timer_handler(Registers *regs)
{
    (void)regs;
    g_tick_count++;
}

/* ---- Delay using PIT ticks (~18.2 Hz) ---- */
static void delay_ticks(uint32_t ticks)
{
    uint32_t start = g_tick_count;
    while (g_tick_count - start < ticks) {
        __asm__ volatile("hlt");
    }
}

/* ---- Startup animation ---- */
static void anim_progress_bar(const char *label, int width, uint8_t col)
{
    VGA_SetColor(VGA_COL_INFO);
    puts("  ");
    puts(label);
    puts(" [");
    VGA_SetColor(col);
    for (int i = 0; i < width; i++) {
        putc('#');
        /* Small delay per block - about 1 tick per 2 blocks */
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
    if (ok) {
        VGA_SetColor(VGA_COL_SUCCESS);
        puts("  [  OK  ] ");
    } else {
        VGA_SetColor(VGA_COL_ERROR);
        puts("  [ FAIL ] ");
    }
    VGA_SetColor(VGA_COL_NORMAL);
    puts(msg);
    puts("\n");
    delay_ticks(1);
}

static void startup_animation(void)
{
    VGA_ClearScreen();

    /* ---- Phase 1: Logo reveal line by line ---- */
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
        puts(logo[i]);
        puts("\n");
        delay_ticks(1);
    }

    VGA_SetColor(VGA_COL_NORMAL);
    puts("\n");

    /* ---- Phase 2: Version text ---- */
    VGA_SetColor(VGA_COL_INFO);
    printf("    OmniOS v%s  |  Build %s  |  x86 32-bit\n", OMNIOS_VERSION, OMNIOS_BUILD);
    VGA_SetColor(VGA_COL_NORMAL);
    puts("\n");

    VGA_DrawHLine('=', VGA_WIDTH, VGA_COL_HEADER);
    puts("\n");

    /* ---- Phase 3: Boot status messages ---- */
    boot_status("Initialising VGA text mode 80x25",     true);
    boot_status("Loading GDT (code/data segments)",      true);
    boot_status("Loading IDT (256 interrupt gates)",      true);
    boot_status("Configuring PIC i8259 (IRQ remap)",     true);
    boot_status("Installing ISR handlers",                true);
    boot_status("Installing IRQ handlers",                true);
    boot_status("Hooking PIT timer (IRQ0, ~18.2 Hz)",   true);
    boot_status("Starting PS/2 keyboard driver (IRQ1)", true);

    puts("\n");

    /* ---- Phase 4: Loading progress bar ---- */
    anim_progress_bar("Loading kernel modules", 40, VGA_COL_SUCCESS);
    anim_progress_bar("Initialising shell    ", 40, VGA_MAKE_COLOR(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));

    puts("\n");

    /* ---- Phase 5: Ready message ---- */
    VGA_DrawHLine('-', VGA_WIDTH, VGA_COL_INFO);
    VGA_SetColor(VGA_COL_SUCCESS);
    puts("  OmniOS v");
    puts(OMNIOS_VERSION);
    puts(" booted successfully!\n");
    VGA_SetColor(VGA_COL_INFO);
    puts("  Type 'help' for commands.  Type 'packages' to see available packages.\n");
    VGA_DrawHLine('-', VGA_WIDTH, VGA_COL_INFO);
    puts("\n");
    VGA_SetColor(VGA_COL_NORMAL);
}

/* ---- Kernel entry ---- */
void kernel_main(void)
{
    HAL_Initialize();
    VGA_Initialize();

    i686_IRQ_RegisterHandler(0, timer_handler);
    KB_Initialize();

    __asm__ volatile("sti");

    startup_animation();

    Shell_Run();

    __asm__ volatile("cli");
    while (1) __asm__ volatile("hlt");
}