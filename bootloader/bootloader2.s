%define FAT_POS 0x500
%define CLUS_POS 0x7E00

%include "def.s"

ORG 0x7C5A
BITS 16

SECTION .text
start:
    call enableA20
    
    mov ax, word [0x7C00 + 14]                ;Load number of reserved sectors 
    ;mov di, FAT_POS
    add ax, si                                 ;Add start of partition offset
    ;push ecx
    ;callFunction loadSector, FAT_POS, 3, ax

    callFunction loadFile, CLUS_POS, si, fileName, FAT_POS 
    ;cx holds number of clusters loaded
    mov eax, SECTOR_SIZE
    mov bl, byte [0x7C00 + 13]             ;Load sector per cluster
    jmp CLUS_POS
    

enableA20:
    mov ax, 0x2400
    int 0x15                ;Enable the A20 line
    ret     

%include "boot.s"


fileName: db "STAGE2  BIN"

