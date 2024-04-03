
BITS 32

extern main
SECTION .text
start:
    mov eax, 16
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    mov ss, eax
    movzx esp, sp
   
    mov esi, string
    mov edi, 0xB8000
    call printString
    
    jmp main


;si holds pointer to string to be printed
printString:                
    mov ah, 0x0F
    mov al, byte [esi]              ;Load char into al
    cmp al, 0                       ;Check for end of string
    je  endPrint

    mov word [edi], ax
    inc esi
    add edi, 2
    jmp printString

endPrint:
    ret


SECTION .data
string: db "Hej from stage2", 0
