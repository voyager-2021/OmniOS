; ============================================================
; OmniOS - crtn.asm
; Provides the epilogue for _init and _fini
; ============================================================

section .init
    ; crtend.o .init content was placed before this
    pop ebp
    ret

section .fini
    ; crtend.o .fini content was placed before this
    pop ebp
    ret