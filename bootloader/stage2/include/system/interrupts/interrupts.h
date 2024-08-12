#ifndef IDT_H
#define IDT_H
#include "definitions.h"

#define INTERRUPT_GATE  0xE
#define TRAP_GATE       0xF

#define ISR __attribute__((naked)) void
#define ISR_RETURN __asm__("iretq")
#define ISR_SAVE_REGS __asm__("push rax\n\
                               push rbx\n\
                               push rcx\n\
                               push rdx\n\
                               push rsi\n\
                               push rdi\n\
                               push rbp\n\
                               push r8\n\
                               push r9\n\
                               push r10\n\
                               push r11\n\
                               push r12\n\
                               push r13\n\
                               push r14\n\
                               push r15\n")

#define ISR_RESTORE_REGS __asm__("pop r15\n\
                                  pop r14\n\
                                  pop r13\n\
                                  pop r12\n\
                                  pop r11\n\
                                  pop r10\n\
                                  pop r9\n\
                                  pop r8\n\
                                  pop rbp\n\
                                  pop rdi\n\
                                  pop rsi\n\
                                  pop rdx\n\
                                  pop rcx\n\
                                  pop rbx\n\
                                  pop rax\n")

void defaultISR(uint8_t vector, uint64_t error);

//https://wiki.osdev.org/Interrupt_Descriptor_Table
void setIDTEntry(uint8_t vectorNum, void (*isrP)(), uint8_t ist, uint8_t gate, uint8_t privilege, bool present);

void initInterrupts(void);

#endif //IDT_H