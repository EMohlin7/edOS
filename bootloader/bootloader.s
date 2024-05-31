%define FAT_POS 0x500
%define CLUS_POS 0x7E00
%define MAX_SIZE 420
%define SECTOR_SIZE 512

ORG 0x7C5A
BITS 16

SECTION .text
start:
    call loadFile
    call enableA20
    ;cx holds number of clusters loaded
    mov eax, SECTOR_SIZE
    mov bl, byte [0x7C00 + 13]             ;Load sector per cluster
    jmp CLUS_POS
    

enableA20:
    mov ax, 0x2400
    int 0x15                ;Enable the A20 line
    ret     


;al holds char to be printed
printChar:                  
    mov ah, 0x0E            ;Teletype output for int 10h
    int 0x10
    ret

;si holds pointer to string to be printed
printString:                
    mov al, [ds:si]         ;Load char into al
    cmp al, 0               ;Check for end of string
    je  endPrint

    call printChar
    inc si
    jmp printString

endPrint:
    ret



loadFile:
    push si
    call loadFAT

    mov eax, dword [0x7C00 + 44]                 ;Load root cluster number
    pop si
    mov di, CLUS_POS
    call loadCluster                                ;Load root root directory

    mov eax, CLUS_POS
    call findFile
    mov ebx, eax                                    ;ebx now holds the address of the file descriptor
    mov ecx, ebx                                    ;Set ecx to the correct address
    add ecx, 0x14
    movzx eax, word [ecx]                           ;Load higher word of cluster number
    shl eax, 16                                     ;Move to higher word of eax
    mov ecx, ebx                                    ;Set ecx to the correct address
    add ecx, 0x1A
    mov ax, word [ecx]                              ;Load lower word of cluster number
    
    mov cx, 0
ldClusters:
    push dx
    mov ebx, eax                                    ;ebx holds cluster number
    mov dx, 4
    mul dx                                         ;eax is now the offset into the FAT
    pop dx
    add eax, FAT_POS                                ;eax is now the postion in memory for FAT entry
    mov eax, dword [eax]                            ;eax holds next cluster number
    and eax, 0x0FFFFFFF                             ;Top four bits are not used
    push eax
    
    ;Calculate destination pos
    push cx
    mov ax, cx
    mov cl, byte [0x7C00 + 13]                    ;Load sector per cluster
    push dx
    mul cl
    mov cx, SECTOR_SIZE                           ;Sector size
    mul cx 
    mov di, CLUS_POS
    add di, ax
    pop dx
    mov eax, ebx
    call loadCluster                                ;Load stage2 bootloader
    pop cx
    pop eax                                         ;pop next cluster number
    inc cx
    cmp eax, 0x0FFFFFF8                             ; >= 0x0FFFFFF8 is the last cluster of a file
    jl ldClusters

    ret

;eax hold cluster number. eax holds return value. si holds first sector of partion
calcSectorOfCluster:
;FirstDataSector = BPB_ResvdSecCnt + (BPB_NumFATs * FATSz)
    push eax
    xor ebx, ebx
    mov word bx, [0x7C00 + 14]                ;Load reserved sector count     
    mov dword eax, [0x7C00 + 36]              ;Load FAT size
    movzx ecx, byte [0x7C00 + 16]             ;Load and zero-extend number of FATs
    mul ecx                                   ;Calc first data sector
    add ebx, eax                              ;Add first data sector
    add ebx, esi                              ;Add start of partition offset. ebx now holds first data sector on the disk

    ;FirstSectorofCluster = ((N – 2) * BPB_SecPerClus) + FirstDataSector;
    pop dword eax                             ;Load cluster number
    movzx ecx, byte [0x7C00 + 13]             ;Load sector per cluster
    
    ;Calculate sector of the first cluster
    sub eax, 2
    mul ecx
    add eax, ebx
    ;dec eax
    ret


loadFAT:
    ;TODO: Fix this. Error when trying to load whole FAT.
    mov dword ecx, 3;[0x7C00 + 36]                 ;Load FAT size
    movzx eax, word [0x7C00 + 14]                ;Load number of reserved sectors 
    mov di, FAT_POS
    add eax, esi                                 ;Add start of partition offset
    ;push ecx
    ldFat:
    call loadSector
    ret
    ;push si
    ;mov si, fatLoaded
    ;call printString
    ;pop si
    ;pop ecx
    ;TODO: Fix so FAT bigger than one word can be loaded.
    

;eax holds cluster number. si holds first sector of partion. di holds address to store cluster
;returns first sector number in eax
loadCluster:
    push di
    push edx
    call calcSectorOfCluster                 ;eax now holds sector number
    xor cx, cx
    mov byte cl, [0x7C00 + 13]               ;Load sector per cluster
    pop edx
    pop di
    call loadSector
    ret

;eax holds number of first sector to load. cx holds number of sectors to load.
;di holds address to store at.
loadSector:                   
    push si
    mov dword [dapl], eax                   ;Set sector number to be loaded
    mov word [dapNumSectors], cx
    mov word [dapDestOffset], di
    mov word [dapDestSegment], 0
    mov si, dap
    push eax
    xor eax, eax
    mov ah, 0x42
    int 0x13                                ;read from disk dl (dl should be the same as this program booted from)
    
    jc error
    pop eax
    pop si
    ret

;eax holds current cluster number
;eax returns new cluster number
;findNextCluster:
;    mov bl, 4
;    mul bl                                   ;Multiply cluster number by 4 since one entry is 4 bytes large
;    add eax, FAT_POS
;    mov dword eax, [eax] 

;eax holds address of folder
;Returns address of file entry. Held in eax. Carry flag set if file found
findFile:
    push si
    mov si, fileName
    sub eax, 32
    mov ecx, 11                                 ;Length of filename
    fileLoop:
    add eax, 32                                 ;Next entry
    mov bl, byte [eax]
    cmp bl, 0                                   ;first byte = 0x0 means end of directory
    je endFind
    cmp bl, 0xE5                                ;first byte = 0xE5 means unused file entry
    je fileLoop

    mov edi, eax                                ;Compare name of current file descriptor
    call strcmp
    jnc fileLoop
    mov ebx, eax                                ;Save eax
    mov esi, eax
    call printString
    mov eax, ebx                                ;Restore eax
    endFind:

    mov si, endDir
    call printString
    mov eax, ebx
    pop si
    ret

;Compare string stored at si with string stored at di. If equal carry flag is set otherwise it is cleared
;ecx holds length of string to be checked
strcmp:
    push ecx
    push si
    clc                                     ;Clear carry
    strcmpLoop:
    cmpsb 
    jne endStrcmp
    loop strcmpLoop
    stc                                     ;Set carry
    endStrcmp:
    pop si
    pop ecx
    ret


error:
    mov si, errorString
    call printString
    jmp $

fileName: db "STAGE2  BIN"

endDir: db "End", 0

errorString: db "Error", 0

align 4
dap:                    ;Disc address packet
    db 16
    db 0
dapNumSectors:    
    dw 1
dapDestOffset:
    dw 0x6000
dapDestSegment:
    dw 0x0
dapl:
    dd 1
daph:
    dd 0

TIMES MAX_SIZE-($-$$)-2 db 0
db 0x55, 0xAA