bits 16
section .STAGE2
extern _start
start:
    jmp _start
    cli
    hlt
