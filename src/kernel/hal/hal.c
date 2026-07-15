/* ============================================================
   OmniOS - Hardware Abstraction Layer
   ============================================================ */
#include "hal.h"
#include "../arch/i686/gdt.h"
#include "../arch/i686/idt.h"
#include "../arch/i686/isr.h"
#include "../arch/i686/irq.h"
#include "../arch/i686/i8259.h"
#include "../arch/i686/vga_text.h"

void HAL_Initialize(void)
{
    /* Order matters: GDT → IDT → PIC → ISR/IRQ → VGA */
    GDT_Initialize();
    IDT_Initialize();
    i8259_Configure(0x20, 0x28, false);   /* remap PIC: IRQ0-7 → 0x20-0x27
                                                        IRQ8-15 → 0x28-0x2F */
    ISR_Initialize();
    IRQ_Initialize();
    VGA_Initialize();
}