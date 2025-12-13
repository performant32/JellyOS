bits 16
org 0x7C00
start:
    cli
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov ax, 0xFFFF
    mov es, ax

    cld
    jmp 0x0000:main
main:
    enter 8

    ; Printing current drive
    ; TODO:
    mov dl, 0xe0
    call get_drive_parameters
    cli
.loop:
    hlt
    jmp .loop
; Parameters
;   dl - drive number
; Registers Changed
;   ah
get_drive_parameters:
    enter 16
    mov [bp - 2], dl

    ; Get Drive Parameters
    ;mov ah, 0x8
    ;mov di, 0   ; Some buggy BIOS
    ;mov dl, 0xE0
    ;int 0x13

    ;jnc .ReadSectors
    ;lea si, [ds:msg_parameter_error]
    ;call print_string
    ;jmp .Done
.ReadSectors:
    sub bp, 2
    ; Settings up DAP
    mov ax, 0
    mov es, ax
    mov word [bp - 16], 0x0010
    mov word [bp - 14], 1
    mov word [bp - 12], 1
    mov word [bp - 10], es
    mov dword [bp - 8], 0
    mov dword [bp - 4], 0

    ; Calling Read Sectors
    mov ah, 0x42
    lea si, [bp-16]
    mov dl, [bp-2]
    int 0x13
    add bp, 2

    ; Checking Errors
    jnc .Success
    lea si, msg_read_sectors_failed
    call print_string
    jmp .Done
.Success:
    lea si, msg_success
    call print_string
.Done:
    leave
    ret
; Parameters used
;   SI - source
; Registers Changed
;   
print_string:
    mov ah, 0x0e
.Loop:
    lodsb
    cmp al, 0
    je .Finish
    int 0x10
    jmp .Loop
.Finish:
    ret
; Parameters sued
;   SI - source
;   dx - n
; Registers Changed
print_string_n:
    push cx
    mov ah, 0x0e
.Loop:
    lodsb
    cmp cx, dx
    je .Finish
    int 0x10
    inc cx
    jmp .Loop
.Finish:
    pop cx
    ret
times 446 - ($-$$) db 0
; Data
%define ENDL 13, 10
msg_parameter_error: db "Error reading drive", ENDL, 0x00
msg_read_sectors_failed: db "Read Sectors Failed", ENDL, 0x00
msg_success: db "Success", ENDL, 0x00
times 510 - ($-$$) db 0
;db 0x55, 0xAA
dw 0xAA55
