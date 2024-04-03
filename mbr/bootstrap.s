%define pEntryOff 446
%define pEntrySize 16
%define pPosOff 0x08
%define pSizeOff 0x0C



BITS 16
ORG 0x6000                  ;The program will be moved to 0x6000 but starts at 0x7C00. Untill the program is moved, labels will not work

SECTION .text
start:
    jmp 0x0:0x7C05          ;Some bioses might load program into 0x7C00:0x0, so enforce segment 0 instead

    mov ax, cs              ;Set other segments to zero
    mov ds, ax 
    mov es, ax
    
    mov si, 0x7C00          ;Set source address of program
    mov di, 0x6000          ;Set destination of program
    mov cx, 0x100           ;Size of boot sector in words
    rep movsw               ;Move cx words from si to di

    jmp 0x0:movedProgram    ;Far jump to moved program since labels are calculated for the moved program and far jumps use absolute address.

movedProgram:
    mov si, string          ;si points to the string to print
    call printString
    call loadBootloader
    jmp 0:0x7C00            ;Jump to the bootloader of the OS

printChar:                  ;al holds char to be printed
    mov ah, 0x0E            ;Teletype output for int 10h
    int 0x10
    ret

printString:                ;si holds pointer to string to be printed
    mov al, [ds:si]         ;Load char into al
    cmp al, 0               ;Check for end of string
    je  endPrint

    call printChar
    inc si
    jmp printString

endPrint:
    ret



loadBootloader:
    mov si, 0x6000+pEntryOff-pEntrySize
    mov cx, 0
    findBootableP:
    cmp cx, 4
    je error                                ;Error if we have checked all four partions without finding a bootable one
    add si, pEntrySize                      ;Set address to the next partition entry

    mov byte bl, [si]                       ;Get active partion byte
    inc cx                                  ;Increase count of checked partitions
    cmp bl, 0x80                            ;Check if partition is active
    jne findBootableP

    mov word [dapNumSectors], 1
    mov word [dapDestOffset], 0x7C00
    mov word [dapDestSegment], 0
    add si, pPosOff                            ;si points to start sector of bootable partition
    mov eax, [si]
    mov dword [dapl], eax                         ;Load sector 
    mov dword [daph], 0

    mov ah, 0x42                                ;set ah to extended read (read from disk using LBA)
    push si                                     ;Save address of partition start entry. (The vbr expects si to hold the start sector number when loaded)
    mov si, dap
    int 0x13                                    ;read from disk dl (dl should be the same as this program booted from)
    jc error
    pop si                                      ;Restore si
    mov si, [si]
    ret

error:
    mov si, errorString
    call printString
    jmp $


string: db "MBR loaded and moved to 0x0:0x6000", 0

errorString: db "Error", 0

align 4
dap:                    ;Disc address packet
    db 16
    db 0
dapNumSectors:    
    dw 1
dapDestOffset:
    dw 0x7C00
dapDestSegment:
    dw 0x0
dapl:
    dd 1
daph:
    dd 0


TIMES pEntryOff-($-$$) db 0