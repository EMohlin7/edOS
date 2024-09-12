%include "def.s"

%define storePos 0x9f00

global memoryMap
global printString
global diskNumber

global kernelSize
global kernelAddress

extern PM
extern loadFile
extern loadFileDescriptor

SECTION .text
BITS 16
start:
    mov sp, 0x400 
    mov [diskNumber], dl 
    mov dword [sectorSize], eax
    mov byte [sectorsPerCluster], bl
    mov word [programClusters], cx

    callFunction loadFileDescriptor, storePos, si, fileName, 0  ;ax now hold address to fileDescriptor 
    test ax, ax
    jnz fileDescriptorFound
    mov si, kernelNotFoundString
    call printString
    jmp $

    fileDescriptorFound:
    mov ecx, dword [eax + 0x1c]  ;get file size
    mov [kernelSize], ecx
    callFunction getMemMap      ;eax now hold valid place for kernel
    mov [kernelAddress], eax
    mov di, si
    mov si, ax
    and si, 0xf ;si is now offset
    shr eax, 4  ;ax is now segment


    callFunction loadFile, si, di, fileName, FAT_POS, ax
    
    ;Set video mode
    ;mov ax, 0x0003
    ;int 10h
    jmp enablePM

getMemMap:
    push ebx
    push edx
    mov di, memoryMap
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
    mov si, memoryMap
    findKernelMemory:
    mov eax, [si + 16]       ;Load memory type
    cmp eax, 1              ;Check if memory is usable
    je usableMemBlock
    nextMemBlock:
    add si, 3*8
    jmp findKernelMemory

    usableMemBlock:
    mov edi, [si + 8]       ;load lower dword length of block
    mov edx, [si + 12]      ;load higher dword
    mov eax, dword [kernelSize]

    test edx, edx
    jnz viableSize
    cmp eax, edi
    jg nextMemBlock

    viableSize:
    mov eax, [si]       ;load lower dword base of block
    mov ebx, [si + 4]   ;load higher dword
    test ebx, ebx
    jnz nextMemBlock    ; if base is above 4GiB we can't load it there

    test edx, edx
    jnz checkBaseOverflow

    cmp eax, 0xA000
    jg checkBaseOverflow  ;The kernel has to be loaded above stage2 in order to not load it where the page table will be initialize later

    mov ebx, 0xA000
    add ebx, dword [kernelSize]
    sub ebx, eax        ;ebx is now the length needed
    cmp edi, ebx
    jl nextMemBlock
    mov eax, 0xA000
    jmp retMem

    checkBaseOverflow:
    mov eax, [si]           ;load lower dword base of block
    mov ebx, [si + 4]       ;load higher dword
    test ebx, ebx
    jnz nextMemBlock    ; if base is above 4GiB we can't load it there
    add eax, [kernelSize]   ;Check for overflow to make sure that the whole kernel fits under 4 GiB
    jc nextMemBlock     
    sub eax, [kernelSize]    

    retMem:
    mov edx, eax
    add edx, dword [kernelSize]
    add edx, 1
    mov ebx, edx
    sub ebx, dword [si]
    mov [si], edx   ;The new base of the memblock is now above the end of the kernel 
    sub [si + 8], ebx ; Change length of block
    pop edx
    pop ebx
    ret  ;eax is a valid storage place for the kernel



enablePM:
    cli
    ;Enable protected mode
    lgdt [PM_GDTDesc]
    mov eax, 0
    or eax, 1               ;Set Protected mode enabled bit
    mov cr0, eax
    jmp 0x0008:PM


;al holds char to be printed
printChar:                  
    mov ah, 0x0E            ;Teletype output for int 10h
    int 0x10
    ret

;si holds pointer to string to be printed
printString:                
    ;push ax
    mov al, [ds:si]         ;Load char into al
    cmp al, 0               ;Check for end of string
    je  endPrint

    call printChar
    inc si
    jmp printString

endPrint:
    ;pop ax
    ret


;50 entries 24 bytes each
memoryMap:
    %rep 50
        dq 0
        dq 0
        dq 0
    %endrep

fileName: db "KERNEL  BIN"
kernelNotFoundString: db "Kernel not found", 0

diskNumber: db 0

kernelSize: dd 0
kernelAddress: dd 0

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