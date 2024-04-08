%define PML4_ADDRESS 0x1000
BITS 32

extern setupPageTable
extern longMode
SECTION .text
start:
    cli
    ;Set data segments to the data segment in the GDT
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
    
    mov eax, PML4_ADDRESS         
    push eax    
    call setupPageTable
    pop eax                 ;Pop parameter from stack

;eax holds PML4-address
elm:
    ;enable PAE
    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax

    ;Modify the efer register
    mov ecx, 0xC0000080          
    rdmsr                        
    or eax, 1 << 8       ; Set the LM-bit 
    wrmsr           

    ;Load address of page table
    mov eax, PML4_ADDRESS
    mov cr3, eax
    
    ;Enable paging. This will enable long mode
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax
    lgdt [GDTDesc]
    jmp 8:setSegments

BITS 64
setSegments:
    ;Set segment registers to data segment 
    mov eax, 16            
    mov ds, eax            
    mov es, eax 
    mov fs, eax   
    mov gs, eax 
    mov ss, eax     
    mov qword [rdi], rax     
    jmp longMode


BITS 32
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
string: db "Stage two loaded", 0

align 8
_GDT:
    ;Null descriptor
    dq 0                   

    ;Code segment
    dw 0xFFFF               ;Limit bits 0-15
    dw 0                    ;Base bits 0-15
    db 0                    ;Base bits 16-23
    db 10011011b            ;Access byte (present, privelige bits 1-2, Descriptor type, Executable, Direction, R/W, Accessed)
    db 10101111b            ;Flag (Granularity, D, Long mode, AVL); Limit bits 16-19
    db 0                    ;base bits 24-31

    ;Data segment
    dw 0xFFFF               ;Limit bits 0-15
    dw 0                    ;Base bits 0-15
    db 0                    ;Base bits 16-23
    db 10010011b            ;Access byte (present, privelige bits 1-2, Descriptor type, Executable, Direction, R/W, Accessed)
    db 10101111b            ;Flag (Granularity, D, Long mode, AVL); Limit bits 16-19
    db 0                    ;base bits 24-31

GDTDesc:
    dw GDTDesc - _GDT - 1
    dq _GDT