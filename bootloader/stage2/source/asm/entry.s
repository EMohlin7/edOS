%define PML4_ADDRESS 0x1000

extern setupPageTable
extern initVMM
extern longMode

global memoryMap

SECTION .text
BITS 16
start:
    mov dword [sectorSize], eax
    mov byte [sectorsPerCluster], bl
    mov word [programClusters], cx
    mov di, memoryMap

    ;Set video mode
    mov ax, 0x0003
    int 10h

    xor ebx, ebx
    ;Map the available memory
    memMap:
        mov edx, 0x534D4150     ;Set edx to magic number
        mov eax, 0xE820         ;Set interrupt argument
        mov ecx, 24             ;Magic number
        int 0x15
        jc memMapComplete
        test ebx, ebx
        jz memMapComplete
        add di, 24
        jmp memMap

memMapComplete:
    cli
    ;Enable protected mode
    lgdt [PM_GDTDesc]
    mov eax, cr0
    or eax, 1               ;Set Protected mode enabled bit
    mov cr0, eax
    jmp 0x0008:PM

;50 entries 24 bytes each
memoryMap:
    %rep 50
        dq 0
        dq 0
        dq 0
    %endrep

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
    
    ;Calculate program size and use it as argument in setupPageTable
    movzx eax, word [programClusters]
    mul byte [sectorsPerCluster]
    mul dword [sectorSize]
    push eax

    mov eax, PML4_ADDRESS         
    push eax    
    call setupPageTable;void setupPageTable(uint32_t PML4, uint32_t programSize)

;Enable long mode
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

    movzx rax, word [programClusters]
    mul byte [sectorsPerCluster]
    mul dword [sectorSize]
    mov rdi, rax

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

programClusters: dw 0
sectorsPerCluster: db 0
sectorSize: dd 0  

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