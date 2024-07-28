#include "system/sysTables/apic.h"
#include "system/sysTables/systemTables.h"
#include "system/sysInfo.h"
#include "memMgmt/vmm.h"
#include "stdlib.h"

//https://wiki.osdev.org/MADT

typedef enum {
    PIC = 0,
    ioAPIC = 1,
    ioAPICiso = 2,      //io APIC interrupt source override
    ioAPICnmi = 3,      //io APIC non-maskable interrupt source
    LAPICnmi = 4,   //local APIC non-maskable interrupts 
    LAPICao = 5,    //local APIC address override
    localx2APIC = 9      

} madtEntryType_t;

typedef struct {
    const SDTHeader_t madtHeader;
    const uint32_t LAPICAddrs;
    const uint32_t flags;
}__attribute__((packed)) madtFirstEntry_t;

typedef struct {
    const uint8_t entryType;
    const uint8_t length;
}__attribute__((packed)) madtEntryHeader_t;


//Entry Type 0: Processor Local APIC
typedef struct {
    const madtEntryHeader_t header;
    const uint8_t processorId;
    const uint8_t APICId;
    const uint32_t flags;
}__attribute__((packed)) LAPICEntry_t;


//Entry Type 1: I/O APIC
typedef struct {
    const madtEntryHeader_t header;
    const uint8_t ioAPICId;
    const uint8_t reserved;
    const uint32_t ioAPICAddrs;
    const uint32_t gsib; //Global system interrupt base
}__attribute__((packed)) ioAPICEntry_t;


//Entry Type 2: I/O APIC Interrupt Source Override
typedef struct {
    const madtEntryHeader_t header;
    const uint8_t busSource;
    const uint8_t IRQSource;
    const uint32_t gsi; //Global system interrupt
    const uint16_t flags;
}__attribute__((packed)) ioAPICisoEntry_t; //io APIC interrupt source override


//Entry type 3: I/O APIC Non-maskable interrupt source
typedef struct {
    const madtEntryHeader_t header;
    const uint8_t nmiSource;
    const uint8_t reserved;
    const uint16_t flags;
    const uint32_t gsb; //Global system interrupt
}__attribute__((packed)) ioAPICnmiEntry_t;


//Entry Type 4: Local APIC Non-maskable interrupts
typedef struct 
{
    const madtEntryHeader_t header;
    const uint8_t processorId;
    const uint16_t flags;
    const uint8_t LINT;
}__attribute__((packed)) LAPICnmiEntry_t;


//Entry Type 5: Local APIC Address Override
typedef struct 
{
    const madtEntryHeader_t header;
    const uint16_t reserved;
    uint64_t pAddrs;
}__attribute__((packed)) LAPICAddrs_t;


//Entry Type 9: Processor Local x2APIC
typedef struct 
{
    const madtEntryHeader_t header;
    const uint16_t reserved;
    const uint32_t x2APICid;
    const uint32_t flags;
    const uint32_t ACPIid;
}__attribute__((packed)) x2LAPICEntry_t;


uint32_t getAPICTimerFrequency(void* APICRegs){
    uint32_t eax = 0x15, ebx, ecx, edx; 
    cpuId(&eax, &ebx, &ecx, &edx);
    printf("Core base: %d. Core max: %d. Bus: %d\n", eax, ebx, ecx);
    return 0;
}

void setupAPIC(SDTHeader_t* madt){
    madtFirstEntry_t* firstEntry = (madtFirstEntry_t*)madt;
    uint8_t* regs = mapPage(firstEntry->LAPICAddrs, R_W | PRESENT | PCD, 0);
    printf("LAPIC addrs: %#x. Timer frequency: %u\n", firstEntry->LAPICAddrs, getAPICTimerFrequency(regs));

    *(uint32_t*)(regs+SPURIOUS_INTERRUPT_VECTOR) = 0x100;
    umapPage(regs);
}

