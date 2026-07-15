; ============================================================
; OmniOS - crti.asm
; Provides the prologue for _init and _fini
; ============================================================

section .init
global _init
_init:
    push ebp
    mov ebp, esp
    ; crtbegin.o .init content will be placed here by the linker

section .fini
global _fini
_fini:
    push ebp
    mov ebp, esp
    ; crtbegin.o .fini content will be placed here by the linker