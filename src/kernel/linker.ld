ENTRY(start)
OUTPUT_FORMAT("elf32-i386")

phys = 0x00100000;

SECTIONS
{
    . = phys;

    .text : {
        __text_start = .;
        *(.text)
    }

    .init : {
        __init_start = .;
        *(.init)
    }

    .fini : {
        __fini_start = .;
        *(.fini)
    }

    .rodata : {
        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
    }

    .data : {
        __data_start = .;
        *(.data)
        *(.data.*)
    }

    .bss : {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    }

    __end = .;
}