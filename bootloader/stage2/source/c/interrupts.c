#include "interrupts.h"
#include "printf.h"
#include "stdlib.h"
#include "sysInfo.h"
#include "vmm.h"

#define KERNEL_CODE_SEGMENT 8
#define EBDA_ADDR 0x80000

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
    __asm__("cli");
    __asm__("hlt");
}


//https://wiki.osdev.org/Interrupt_Descriptor_Table
void setIDTEntry(uint8_t vectorNum, void* isrP, uint8_t ist, uint8_t gate, uint8_t privilege, bool present){
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

void initIDT(void){
    memset(vectors, NULL, sizeof(vectors));

    for(int i = 0; i < 32; ++i){
        setIDTEntry(i, isrTable[i], 0, INTERRUPT_GATE, 0, true);
    }

    idtr.size = sizeof(vectors);
    idtr.base = (uint64_t)vectors;
    printf("Loading IDT at address %p\n", (void*)vectors);
    __asm__("lidt %0" :/*No output*/ : "m" (idtr));
}


//https://wiki.osdev.org/RSDP
typedef struct {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdtAddress;
} __attribute__ ((packed)) RSDP;

typedef struct{
    RSDP rsdp;

    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
} __attribute__ ((packed)) XSDP;


static bool checkRSDPChecksum(RSDP* rsdp){
    uint64_t val = 0;
    for(size_t i = 0; i < sizeof(RSDP); ++ i){
        val += ((uint8_t*)rsdp)[i];
    }
    if((uint8_t)val != 0){
        return false;
    }
    else if(rsdp->revision < 2){
        return true;
    }

    //Check extended table aswell
    val = 0;
    for(size_t i = 0; i < sizeof(XSDP); ++ i){
        val += ((uint8_t*)rsdp)[i];
    }
    if((uint8_t)val != 0){
        return false;
    }

    return true;
}

/// @brief Finds the rsdp
/// @return Returns the root system directory pointer. NULL if the pointer couldn't be found or the checksum was invalid
static RSDP* findRSDP(void){
    char id[8] = "RSD PTR ";
    //Search first KB of EBDA for the RSDP. The RSDP is 16 byte aligned
    for(uint64_t ptr = EBDA_ADDR; ptr < EBDA_ADDR+1024; ptr += 16){
        if(memcmp((RSDP*)ptr, id, 8) == 0 && checkRSDPChecksum((RSDP*)ptr))  
            return (RSDP*)ptr;
    }
    
    for(uint64_t ptr = 0xE0000; ptr < 0xFFFFF; ptr +=16){
        if(memcmp((RSDP*)ptr, id, 8) == 0 && checkRSDPChecksum((RSDP*)ptr))
            return (RSDP*)ptr;
    }

    return NULL;
}


void setupAPIC(void){
    uint32_t edx, _;
    cpuId(1, &_, &edx, &_);
    //Bit 9 in edx tell if processor has local APIC
    if(edx & (1 << 8)){
        printf("Local APIC supported\n");
    }else{
        printf("Local APIC is not supported. Halting...");
        halt();
    }

    //Disable PIC
    __asm__("mov al, 0xFF\n\t"  \
            "outb 0x21, al\n\t" \
            "outb 0xA1, al"     \
            : /*No output*/ : /*No input*/ : "al");

    
    RSDP* rsdp = findRSDP();
    if(rsdp == NULL){
        printf("No RSDP found, halting...\n");
        halt();
    }

    if(rsdp->revision == 0){
        printf("ACPI version 1.0\n");
    }else{
        printf("ACPI version 2.0\n");
    }

    printf("rsdt address: %#x\n", rsdp->rsdtAddress);
    //Make sure the OEMID is null ended
    char oemid[7];
    oemid[6] = NULL;
    memcpy(oemid, rsdp->OEMID, 6); 
    printf("OEMID: %s\n", oemid);

    void* a = mapPage(rsdp->rsdtAddress, PRESENT | R_W, 0);
    printf("Mapped address: %p\n", a);
}

void initInterrupts(void){
    initIDT();
    setupAPIC();

    //Enable interrupts
    __asm__("sti");
}
