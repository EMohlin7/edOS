%define PML4_ADDRESS 0x1000

extern setupPageTable
extern longMode
SECTION .text
BITS 16
start:
    cli
    ;TODO: Check if already in protected mode first
    ;Enable protected mode
    lgdt [PM_GDTDesc]
    mov eax, cr0
    or eax, 1               ;Set Protected mode enabled bit
    mov cr0, eax
    jmp 0x0008:PM

BITS 32
PM:
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

;Enable long mode
;eax holds PML4-address
elm:
    ;enable PAE
    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax

    ;Modify the efer register and set the LME-bit
    mov ecx, 0xC0000080          
    rdmsr                        
    or eax, 1 << 8        
    wrmsr           

    ;Load address of page table
    mov eax, PML4_ADDRESS
    mov cr3, eax
    
    ;Enable paging. This will enable long mode
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax
    lgdt [LM_GDTDesc]
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

align 4
PM_GDT:
    ;Null descriptor
    dq 0                   

    ;Code segment
    dw 0xffff               ;Limit bits 0-15
    dw 0                    ;Base bits 0-15
    db 0                    ;Base bits 16-23
    db 10011011b            ;Access byte (present, privelige bits 1-2, Descriptor type, Executable, Direction, R/W, Accessed)
    db 11001111b            ;Flag (Granularity, Size, Long mode, Reserved); Limit bits 16-19
    db 0                    ;base bits 24-31

    ;Data segment
    dw 0xffff               ;Limit bits 0-15
    dw 0                    ;Base bits 0-15
    db 0                    ;Base bits 16-23
    db 10010011b            ;Access byte (present, privelige bits 1-2, Descriptor type, Executable, Direction, R/W, Accessed)
    db 11001111b            ;Flag (Granularity, Size, Long mode, Reserved); Limit bits 16-19
    db 0                    ;base bits 24-31

PM_GDTDesc:
    dw PM_GDTDesc - PM_GDT - 1
    dd PM_GDT


align 8
LM_GDT:
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

LM_GDTDesc:
    dw LM_GDTDesc - LM_GDT - 1
    dq LM_GDT