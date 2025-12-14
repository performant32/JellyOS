bits 16
db "Hello Boot2", 0x0
org 0x7D00
main:
    mov ah, 0x0E
    mov al, '6'
    int 0x10
    jmp main
