

extern defaultISR;(uint8_t vector, uint64_t error)

%macro ISR_SAVE_REGS 0
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro ISR_RESTORE_REGS 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

%macro isrEntry 1
isr_%1:
    ISR_SAVE_REGS
    mov rdi, %1
    mov rsi, 0
    call defaultISR
    ISR_RESTORE_REGS
    iretq
%endmacro
%macro isrEntryErrCode 1
isr_%1:
    ISR_SAVE_REGS
    mov rdi, %1
    pop rsi
    call defaultISR
    ISR_RESTORE_REGS
    iretq
%endmacro

isrEntry 0
isrEntry 1
isrEntry 2
isrEntry 3
isrEntry 4
isrEntry 5
isrEntry 6
isrEntry 7
isrEntryErrCode 8
isrEntry 9
isrEntryErrCode 10
isrEntryErrCode 11
isrEntryErrCode 12
isrEntryErrCode 13
isrEntryErrCode 14
isrEntry 15
isrEntry 16
isrEntryErrCode 17
isrEntry 18
isrEntry 19
isrEntry 20
isrEntry 21
isrEntry 22
isrEntry 23
isrEntry 24
isrEntry 25
isrEntry 26
isrEntry 27
isrEntry 28
isrEntry 29
isrEntryErrCode 30
isrEntry 31

%macro tableEntry 1
    dq isr_%1
%endmacro

global isrTable
align 8
isrTable:
    %assign i 0
    %rep    32             ;32 Predetermined interrupts
        tableEntry i
    %assign i i+1
    %endrep
