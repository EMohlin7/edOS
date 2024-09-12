%define PML4_ADDRESS 0x1000
%define PAGE_SIZE 0x1000

global PM

extern kernelSize
extern kernelAddress

extern mapKernel
extern LM

SECTION .text
BITS 32
PM:
    movzx esp, sp
    mov ebp, esp
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
    call printString32
    

    push ebp
    mov ebp, esp
    
    mov eax, [kernelAddress]
    push eax
    mov eax, [kernelSize]
    push eax
    mov eax, PML4_ADDRESS         
    push eax   

    call setupPageTable
    mov esp, ebp
    pop ebp

    jmp elm
  
;void setupPageTable(uint32_t PML4, uint32_t kernelSize, uint32_t kernelPhysicalMemAddrs)
setupPageTable: 
    mov edi, [ebp - 12]
    mov ecx, PAGE_SIZE * 4 / 4
    xor eax, eax
    rep stosd           ;Initialize the page tables to zero

    mov eax, [ebp - 12]
    ;Map the stage 2 bootloader, i.e this program, one to one so that the addresses still work when entering long mode
    mov ecx, 3
    mov edx, 0x2003
    linkStage2:
    mov dword [eax], edx  
    add eax, PAGE_SIZE
    add edx, PAGE_SIZE
    loop linkStage2 

    ;Map the first 2 MiB one to one so that the addresses in this program keep working
    mov edx, 3 
    mov ecx, 512
    mov eax, [ebp-12] 
    add eax, PAGE_SIZE*3
    mapPT:
    mov dword [eax], edx
    add eax, 8
    add edx, PAGE_SIZE
    loop mapPT

    mov eax, [bp-12]
    mov edx, eax
    or edx, 3
    mov dword [eax + 511*8], edx    ;Point last entry of PML4 to itself to allow for recursive mapping

    ret

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
    mov ecx, eax
    mov edx, ecx
    xor ecx, ecx
    mov eax, ecx
    jmp 8:LM


;si holds pointer to string to be printed
printString32:                
    mov ah, 0x0F
    mov al, byte [esi]              ;Load char into al
    cmp al, 0                       ;Check for end of string
    je  endPrint32

    mov word [edi], ax
    inc esi
    add edi, 2
    jmp printString32

endPrint32:
    ret



SECTION .data

string: db "Stage two loaded", 0



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


