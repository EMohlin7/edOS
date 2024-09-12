%include "def.s"

BITS 16
; si hold first sector of partion
; void loadFile(word storeAddress, word firstPartionSector, word fileName, word fatPos)
loadFile: ;si = storeAddress, di = firstPartition, cx = fileName, [bp-2] = fatPos
    push ebx
    push edx
    mov ebx, dword [0x7C00 + 44]                ;Load root cluster number
    callFunction loadCluster, si, di, bx        ;Load root directory
    callFunction findFile, si, cx               ;ax hold address of file descriptor     
    mov bx, ax                                  ;bx now hold address of file descriptor
    mov ax, word [bx + 14h]                     ;Load higher word of cluster number
    shl eax, 16
    mov ax, word [bx + 1ah]                     ;Load lower word of cluster number
    
    mov cx, 0                                   ;Use to increment loop
ldClusters:
    mov edx, eax                                ;edx hold current cluster number
    mov bx, 4
    mul bx                                      ;eax is now the offset into the fat
    add ax, word [bp-2]                         ;eax is now the postion in memory for FAT entry
    mov eax, dword [eax]                        ;eax hold next cluster number
    and eax, 0x0FFFFFFF                         ;Top four bits are not used
    push eax

    mov ax, cx
    movzx bx, byte [0x7C00 + 13]                ;Load sector per cluster
    mul bx
    mov bx, SECTOR_SIZE
    mul bx
    add ax, si                                  ;ax now hold store address of the cluster
    callFunction loadCluster, ax, di, dx        ;Load the cx'th cluster of file
    pop eax                                     ;pop next cluster number
    inc cx
    cmp eax, 0x0FFFFFF8                         ; >= 0x0FFFFFF8 is the last cluster of a file
    jl ldClusters

    pop edx
    pop ebx
    ret

;void loadCluster(word storeAddress, word firstPartitionSector, word clusterNumber)
loadCluster:
    callFunction calcSectorOfCluster, cx, di      ;eax now holds sector number
    mov ecx, eax
    movzx di, byte [0x7C00 + 13]                  ;Load sector per cluster  
    callFunction loadSector, si, di, cx
    ret

;void loadSector(word storeAddress, word sectorsToLoad, dword firstSectorToLoad) 
loadSector:
    mov dword [dapl], ecx                   ;Set sector number to be loaded
    mov word [dapNumSectors], di
    mov word [dapDestOffset], si
    mov word [dapDestSegment], 0
    mov si, dap                     ;si have to point to the dap to load
    mov ah, 42h                     ;Extended read
    int 13h                         ;read from disk dl (dl should be the same as this program booted from)

    jc error
    ret

;dword calcSectorOfCluster(word clusterNumber, word firstPartionSector) 
calcSectorOfCluster:
    push dx
    xor ecx, ecx
    mov cx, word [0x7C00 + 14]      ;Load reserved sector count 
    mov eax, dword [0x7C00 + 36]    ;Load FAT size
    mul ecx                         ;Calc first data sector
    add eax, edi                    ;Add start of partition offset. eax now holds first data sector on the disk

    ;FirstSectorofCluster = ((N – 2) * BPB_SecPerClus) + FirstDataSector;
    mov cx, si                      ;cx = clusterNumber
    sub cx, 2                       
    push eax                        ;Save firstDataSector
    movzx eax, byte [0x7C00 + 13]   ;Load sector per cluster
    mul ecx
    pop edi
    add eax, edi 
    pop dx
    ret
    
;word findFile(word rootDirectoryAddress, word fileName) Return address of file descriptor.  Carry flag set if file found
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
    jnc fileLoop

    push si
    call printString
    endFind:
    mov si, endDir
    call printString
    pop si
    ret

;void strcmp(word string1, word string2, word length)
strcmp:
    clc                                     ;Clear carry
    strcmpLoop:
    cmpsb 
    jne endStrcmp
    loop strcmpLoop
    stc                                     ;Set carry
    endStrcmp:
 
    ret

;al holds char to be printed
printChar:                  
    mov ah, 0x0E            ;Teletype output for int 10h
    int 0x10
    ret

;si holds pointer to string to be printed
printString:                
    push ax
    mov al, [ds:si]         ;Load char into al
    cmp al, 0               ;Check for end of string
    je  endPrint

    call printChar
    inc si
    jmp printString

endPrint:
    pop ax
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
