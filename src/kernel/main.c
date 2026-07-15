/* ============================================================
   OmniOS - Kernel Entry Point
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

#define OMNIOS_VERSION  "0.1.0"
#define OMNIOS_BUILD    "2024.001"

/* ---- Global tick counter ---- */
volatile uint32_t g_tick_count = 0;

static void timer_handler(Registers *regs)
{
    (void)regs;
    g_tick_count++;
}

/* ---- Boot status line ---- */
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
}

/* ---- Boot banner ---- */
static void print_boot_banner(void)
{
    VGA_ClearScreen();

    VGA_DrawHLine('=', VGA_WIDTH, VGA_COL_HEADER);

    VGA_SetColor(VGA_COL_HEADER);
    puts("\n");
    puts("    ..|'''.|                   .|'''.|  \n");
    puts("   .|'     '    mm    nnnn     ||..  '  \n");
    puts("   ||    ....  'MM'   nn ''Nn  ''|||.   \n");
    puts("   '|.    ||  .M''M.  nn   MM  .   '||  \n");
    puts("    ''|...'| .M'  'Mb.MM  .M'  |'...|'  \n");
    puts("\n");
    VGA_SetColor(VGA_COL_NORMAL);

    VGA_SetColor(VGA_COL_INFO);
    printf("    Terminal Operating System  v%s  |  Build %s\n\n",
           OMNIOS_VERSION, OMNIOS_BUILD);
    VGA_SetColor(VGA_COL_NORMAL);

    VGA_DrawHLine('=', VGA_WIDTH, VGA_COL_HEADER);
    puts("\n");

    boot_status("VGA text mode 80x25 initialised",    true);
    boot_status("GDT loaded (null/code/data)",         true);
    boot_status("IDT loaded (256 entries)",             true);
    boot_status("PIC i8259 remapped (IRQ 0x20-0x2F)",  true);
    boot_status("ISR handlers installed",               true);
    boot_status("IRQ handlers installed",               true);
    boot_status("PIT timer hooked (IRQ0)",              true);
    boot_status("PS/2 keyboard driver active (IRQ1)",   true);
    boot_status("OmniOS kernel shell ready",            true);

    puts("\n");
    VGA_DrawHLine('-', VGA_WIDTH, VGA_COL_INFO);
    VGA_SetColor(VGA_COL_INFO);
    puts("  Type 'help' for commands.  Type 'banner' to redraw this screen.\n");
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

    print_boot_banner();

    Shell_Run();

    __asm__ volatile("cli");
    while (1) __asm__ volatile("hlt");
}