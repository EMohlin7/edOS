#include "system/interrupts/interrupts.h"
#include "stdlib.h"
#include "system/sysInfo.h"
#include "system/sysTables/systemTables.h"
#include "system/interrupts/apic.h"
#include "system/time/hpet.h"
#include "memMgmt/vmm.h"

#define KERNEL_CODE_SEGMENT 8

typedef struct {
	uint16_t	size;
	uint64_t	base;
} __attribute__((packed)) idtr_t;

typedef struct {
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP
	uint8_t     flags;        // Type and attributes
	uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
	uint32_t    isr_high;     // The higher 32 bits of the ISR's address
	uint32_t    reserved;     // Set to zero
} __attribute__((packed)) idt_entry_t;



static idtr_t idtr;
static idt_entry_t vectors[256];
extern void* isrTable[];


void defaultISR(uint8_t vector, uint64_t error){
    printf("Interrupt number: %hhu with error code: %lu\n", vector, error);
}


//https://wiki.osdev.org/Interrupt_Descriptor_Table
void setIDTEntry(uint8_t vectorNum, void (*isrP)(), uint8_t ist, uint8_t gate, uint8_t privilege, bool present){
    uint64_t isr = (uint64_t)isrP;
    idt_entry_t* entry = vectors+vectorNum;

    entry->isr_low      = (isr & 0xFFFF);
    entry->kernel_cs    = KERNEL_CODE_SEGMENT;
    entry->ist          = ist & 7;
    entry->flags        = (present != 0) << 7 | ((privilege & 3) << 6) | (0 << 5) | (gate & 0xF);
    entry->isr_mid      = (isr >> 16) & 0xFFFF;
    entry->isr_high     = (isr >> 32) & 0xFFFFFFFF; 
    entry->reserved     = 0;
}

void initIDT(){
    memset(vectors, 0, sizeof(vectors));

    for(int i = 0; i < 32; ++i){
        setIDTEntry(i, isrTable[i], 0, INTERRUPT_GATE, 0, true);
    }

    idtr.size = sizeof(vectors);
    idtr.base = (uint64_t)vectors;
    printf("Loading IDT at address %p\n", (void*)vectors);
    __asm__("lidt %0" :/*No output*/ : "m" (idtr));
}

lapicSetup_t* lapicSetup;

ISR hpetInter(){
    ISR_SAVE_REGS;

    printf("timer\n");
    lapicWriteReg(lapicSetup->registerAddress, LAPIC_EOI_REGISTER, 0);

    ISR_RESTORE_REGS;
    ISR_RETURN;
}

void setupInterrupts(){
    uint32_t eax = 1, edx, _;
    cpuId(&eax, &_, &edx, &_);
    //Bit 9 in edx tell if processor has local APIC
    if(edx & (1 << 8)){
        printf("Local APIC supported\n");
    }else{
        printf("Local APIC is not supported. Halting...");
        halt();
    }

    RSDP_t* rsdp = findRSDP();
    if(rsdp == NULL){
        printf("No RSDP found, halting...");
        halt();
    }

    printf("ACPI version %d\n", rsdp->revision + 1);
    
    //Make sure the OEMID is null terminated before printing
    makeNullTerminated(oemid, rsdp->OEMID);
    printf("OEMID: %s\n", oemid);

    SDTHeader_t* rsdt = getRSDT(rsdp); //Returns XSDT if it exists
    if(rsdt == NULL){
        printf("No RSDT found, halting...");
        halt();
    }

    SDTHeader_t* hpetTable = findSDT(rsdt, "HPET");
    if(hpetTable == NULL){ //TODO: Use PIT instead
        printf("No HPET found, halting...");
        halt();
    }

    
    hpet_t* hpet = hpetInit(hpetTable);
    bool legacy = hpetSetLegacy(hpet, true);
    uint8_t timer = 0;
    uint8_t ioapicEntry = legacy ? 2 : hpetGetRoute(hpet, timer, false);
    hpetStartOneShot(hpet, timer, hpet->frequency*2, ioapicEntry);
    SDTHeader_t* madt = findSDT(rsdt, "APIC");
    if(madt){
        makeNullTerminated(madtSign, madt->signature);
        printf("APIC table found. Sign: %s. Length: %d\n", madtSign, madt->length);
        lapicSetup = lapicInit(madt);
        ioapicSetup_t* test = ioapicInit(madt);
        ioapicRedirEntry_t entry = {
            .activeLow = true,
            .deliveryMode = 0,
            .irqVector = 33,
            .lapicId = 0,
            .levelSens = false,
            .logicAddrs = false,
            .masked = false
        };
        ioapicSetRedirEntry(test->ioapics, ioapicEntry, entry);
    }
    setIDTEntry(33, hpetInter, 0, INTERRUPT_GATE, 0, true);
    hpetEnable(hpet);
    
}

static void disablePIC(){
    //Disable PIC
    __asm__("mov al, 0xFF\n\t"  \
            "outb 0x21, al\n\t" \
            "outb 0xA1, al"     \
            : /*No output*/ : /*No input*/ : "al");

    //Disable PIT
    __asm__("mov al, 0x10\n\t" \
            "outb 0x43, al\n\t" \
            "mov al, 1\n\t" \
            "outb 0x40, al"\
            : : : "al");
}

void initInterrupts(void){
    initIDT();
    disablePIC();
    //Enable interrupts
    __asm__("sti");
    setupInterrupts();
}
