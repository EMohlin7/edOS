%include "def.s"

global loadFile
global loadFileDescriptor

extern printString
extern diskNumber

SECTION .text
BITS 16

;void loadFile(word storeOffset, word firstPartionSector, word fileName, word storeSegment)
loadFileDescriptor:
    push ebx
    push dx
    mov dx, [bp-2]
    mov ebx, dword [0x7C00 + 44]                ;Load root cluster number
    callFunction loadCluster, si, di, bx, dx        ;Load root directory
    callFunction findFile, si, cx               ;ax hold address of file descriptor  
    pop dx
    pop ebx

    ret
; si hold first sector of partion
; void loadFile(word storeAddress, word firstPartionSector, word fileName, word fatPos, word storeSegment)
loadFile: ;si = storeAddress, di = firstPartition, cx = fileName, [bp-4] = fatPos
    push ebx
    push edx
    callFunction loadFileDescriptor, 0x9f00, di, cx, 0 ;ax hold address of file descriptor 
    
    mov bx, ax                                  ;bx now hold address of file descriptor
    mov ax, word [bx + 14h]                     ;Load higher word of cluster number
    shl eax, 16
    mov ax, word [bx + 1ah]                     ;Load lower word of cluster number
    
    mov cx, 0                                   ;Use to increment loop
ldClusters:
    mov edx, eax                                ;edx hold current cluster number
    push dx
    mov bx, 4
    mul bx                                      ;eax is now the offset into the fat
    add ax, word [bp-4]                         ;eax is now the postion in memory for FAT entry
    mov eax, dword [eax]                        ;eax hold next cluster number
    and eax, 0x0FFFFFFF                         ;Top four bits are not used
    pop bx
    push eax

    movzx eax, cx
    movzx dx, byte [0x7C00 + 13]                ;Load sector per cluster
    mul dx
    mov dx, SECTOR_SIZE
    mul dx
    add ax, si                                  ;ax now hold store address of the cluster
    mov dx, [bp-2]
    callFunction loadCluster, ax, di, bx, dx        ;Load the cx'th cluster of file
    pop eax                                     ;pop next cluster number
    inc cx
    cmp eax, 0x0FFFFFF8                         ; >= 0x0FFFFFF8 is the last cluster of a file
    jl ldClusters

    pop edx
    pop ebx
    ret

;void loadCluster(word storeOffset, word firstPartitionSector, word clusterNumber, word storeSegment)
loadCluster:
    callFunction calcSectorOfCluster, cx, di      ;eax now holds sector number
    movzx ecx, ax
    movzx di, byte [0x7C00 + 13]                  ;Load sector per cluster  
    push dx
    mov dx, [bp-2]
    callFunction loadSector, si, di, cx, dx
    pop dx
    ret

;void loadSector(word storeOffset, word sectorsToLoad, dword firstSectorToLoad, word storeSegment) 
loadSector:
    push dx
    mov dword [dapl], ecx                   ;Set sector number to be loaded
    mov word [dapNumSectors], di
    mov word [dapDestOffset], si
    mov dx, [bp-2]
    mov word [dapDestSegment], dx
    movzx dx, byte [diskNumber]
    mov si, dap                     ;si have to point to the dap to load
    mov ah, 42h                     ;Extended read
    int 13h                         ;read from disk dl (dl should be the same as this program booted from)

    pop dx
    jc error
    ret

;dword calcSectorOfCluster(word clusterNumber, word firstPartionSector) 
calcSectorOfCluster:
    push dx
    movzx ecx, word [0x7C00 + 14]   ;Load reserved sector count 
    mov eax, dword [0x7C00 + 36]    ;Load FAT size
    movzx edx, byte [0x7C00 + 16]   ;Load number of FATs
    mul edx                         ;Calc first data sector
    add eax, ecx
    add eax, edi                    ;Add start of partition offset. eax now holds first data sector on the disk

    ;FirstSectorofCluster = ((N – 2) * BPB_SecPerClus) + FirstDataSector;
    xor ecx, ecx
    mov cx, si                      ;cx = clusterNumber
    sub cx, 2                       
    push eax                        ;Save firstDataSector
    movzx eax, byte [0x7C00 + 13]   ;Load sector per cluster
    mul ecx
    pop edi
    add eax, edi 
    pop dx
    ret
    
;word findFile(word rootDirectoryAddress, word fileName) Return address of file descriptor. null if not found
findFile:
    sub si, 32
    fileLoop:
    add si, 32                   ;Next entry
    mov bl, byte [si]
    cmp bl, 0                    ;first byte = 0x0 means end of directory
    je endFind
    cmp bl, 0xE5                 ;first byte = 0xE5 means unused file entry
    je fileLoop
    callFunction strcmp, si, di, 11
    test ax, ax
    jz fileLoop

    push si
    call printString
    pop si
    mov ax, si
    jmp retFind
    endFind:
    mov si, endDir
    call printString
    mov ax, 0
    retFind:
    ret

;bool strcmp(word string1, word string2, word length)
strcmp:
    mov ax, 0
    strcmpLoop:
    cmpsb 
    jne endStrcmp
    loop strcmpLoop
    mov ax, 1
    endStrcmp:
    ret


error:
    mov si, errorString
    call printString
    jmp $

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