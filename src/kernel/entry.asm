; ============================================================
; OmniOS - Kernel Entry Point (assembly)
; Defines the 'start' symbol that the linker expects.
; Sets up the stack and calls kernel_main().
; ============================================================

[bits 32]

section .text

extern kernel_main
extern _init
extern _fini

global start
start:
    ; Set up stack (8KB)
    mov esp, stack_top

    ; Call global constructors
    call _init

    ; Call kernel main
    call kernel_main

    ; Call global destructors (if kernel_main ever returns)
    call _fini

    ; Halt if kernel_main returns
.halt:
    cli
    hlt
    jmp .halt

; ============================================================
; Stack - 8KB
; ============================================================
section .bss
align 16
stack_bottom:
    resb 8192
stack_top: