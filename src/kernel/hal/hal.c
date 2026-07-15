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
    i686_GDT_Initialize();
    i686_IDT_Initialize();
    i8259_Configure(0x20, 0x28, false);
    i686_ISR_Initialize();
    i686_IRQ_Initialize();
    VGA_Initialize();
}