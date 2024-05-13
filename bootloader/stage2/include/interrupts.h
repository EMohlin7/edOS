#ifndef IDT_H
#define IDT_H
#include <stdint.h>

#define INTERRUPT_GATE  0xE
#define TRAP_GATE       0xF

//https://wiki.osdev.org/Interrupt_Descriptor_Table
void setIDTEntry(uint8_t vectorNum, void* isrP, uint8_t ist, uint8_t gate, uint8_t privilege, _Bool present);

void initInterrupts(void);

#endif //IDT_H