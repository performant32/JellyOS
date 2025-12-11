bits 16
org 0x7C00

start:
    cli
    jmp 0x0000:main
main:
    mov ah, 0x0e
    mov al, 'w'
    int 0x10
    cli
    hlt
times 446 - ($-$$) db 0
times 510 - ($-$$) db 0
;db 0x55, 0xAA
dw 0xAA55
