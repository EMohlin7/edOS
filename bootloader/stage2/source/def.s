%ifndef DEF_H
%define DEF_H

%define arg1 si
%define arg2 di
%define arg3 cx
;return values in ax

%macro args 2
    %ifnidn %1, %2
        mov %1, %2
    %endif
%endmacro

%macro callFunction 1-*
    push bp
    push si
    push di
    push cx
    mov bp, sp
    
        
    %assign i %0
    %if %0 > 4
        %rep 100
            %assign i i-1
            %rotate -1      ;Rotate the argument list so that the function arguments are pushed from rigth to left
            push %1
            %if i == 4
                %exitrep
            %endif
        %endrep
    %endif

    %if %0 > 1
        %rep 3
            %assign i i-1
            %rotate -1
            args arg %+ i, %1 
            %if i == 1
                %exitrep
            %endif
        %endrep

        %rotate -1
    %endif
    call %1
    mov sp, bp
    pop cx
    pop di
    pop si
    pop bp
%endmacro

%define FAT_POS 0x500
%define CLUS_POS 0x7E00
%define MAX_SIZE 420
%define SECTOR_SIZE 512


%endif