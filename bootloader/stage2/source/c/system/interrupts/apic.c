#include "system/interrupts/apic.h"
#include "system/sysTables/systemTables.h"
#include "system/sysInfo.h"
#include "memMgmt/vmm.h"
#include "stdlib.h"
#include "msr.h"
#include "system/interrupts/interrupts.h"

//https://wiki.osdev.org/MADT
//https://uefi.org/sites/default/files/resources/ACPI_Spec_6_5_Aug29.pdf

typedef enum {
    LAPIC_ENTRY = 0,
    IOAPIC_ENTRY = 1,
    IOAPIC_ISO_ENTRY = 2,      //io APIC interrupt source override
    IOAPIC_NMI_ENTRY = 3,      //io APIC non-maskable interrupt source
    LAPIC_NMI_ENTRY = 4,       //local APIC non-maskable interrupts 
    LAPIC_AO_ENTRY = 5,        //local APIC address override
    X2APIC_ENTRY = 9      

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
    const uint32_t gsiBase; //Global system interrupt base
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
    const uint16_t flags;
    const uint32_t gsi; //Global system interrupt
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
    const uint64_t pAddrs;
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

//http://web.archive.org/web/20161130153145/http://download.intel.com/design/chipsets/datashts/29056601.pdf

#define IOAPIC_ID_REGISTER 0            //RW
#define IOAPIC_VERSION_REGISTER 1       //R only
#define IOAPIC_ARBITRATION_REGISTER 2   //R only
#define IOAPIC_REDIR_TABLE_REGISTER(index) (0x10 + 2*index)

void lapicWriteReg(void* apicBase, uint32_t reg, uint32_t val){
    uint8_t* base = apicBase;
    *(uint32_t*)(base+reg) = val;
}

uint32_t lapicReadReg(const void* apicBase, uint32_t reg){
    const uint8_t* base = apicBase;
    return *(uint32_t*)(base+reg);
}

void ioapicWriteReg(void* ioapicBase, uint8_t reg, uint64_t val){
    uint32_t low = val & UINT32_MAX;
    uint32_t high = val >> 32;

    uint8_t* ioregsel = (uint8_t*)ioapicBase;
    volatile uint32_t* iowin = (uint32_t*)(ioregsel + 0x10);
    *ioregsel = reg;
    *iowin = low;

    //These registers are 64bit wide but only 32bits can be written at a time, so they act as two 32bit register next to eachother
    if(reg > 2){
        *ioregsel = reg+1;
        *iowin = high; 
    }
}

uint64_t ioapicReadReg(const void* ioapicBase, uint8_t reg){
    uint8_t* ioregsel = (uint8_t*)ioapicBase;
    volatile uint32_t* iowin = (uint32_t*)(ioregsel + 0x10);

    *ioregsel = reg;
    uint32_t low = *iowin;
    uint64_t high = 0;
    if(reg > 2){
        *ioregsel = reg+1;
        high = *iowin;
        high <<= 32;
    }
    return high | low;
}

static ISR spuriousInter(){
    ISR_SAVE_REGS;

    printf("spurious interrupt\n");

    ISR_RESTORE_REGS;
    ISR_RETURN;
}


static void enableLAPIC(const lapicSetup_t* lapic){
    uint64_t msr = readMSR(0x1b);
    if(!(msr & 0x800)) //Enable the APIC if it is disabled
        writeMSR(0x1b, msr | 0x800);

    setIDTEntry(0xff, spuriousInter, 0, INTERRUPT_GATE, 0, true);
    lapicWriteReg(lapic->registerAddress, LAPIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER, 0x1ff);
}



/// @brief Set a redirection entry in an IOAPIC.
/// @param ioapic Pointer to the IOAPIC whose entry will be changed.
/// @param entryNum The number of the entry to change.
/// @param entry The new value of the entry.
/// @return True if the entry was successfully set. False if the "entryNum" was invalid.
bool ioapicSetRedirEntry(ioapic_t* ioapic, uint8_t entryNum, ioapicRedirEntry_t entry){
    if(ioapic->numEntries <= entryNum)
        return false;

    uint64_t val = 0;
    val |= (uint64_t)entry.lapicId << 56;
    val |= (entry.masked     & 1)  << 16;
    val |= (entry.levelSens  & 1)  << 15;
    val |= (entry.activeLow  & 1)  << 13;
    val |= (entry.logicAddrs & 1)  << 11;
    val |= entry.deliveryMode      << 8;
    val |= entry.irqVector;

    ioapicWriteReg(ioapic->address, IOAPIC_REDIR_TABLE_REGISTER(entryNum), val);

    ioapic->entries[entryNum] = entry.irqVector;

    return true;
}

static void ioapicGetNumbers(const ioAPICEntry_t* ioapicEntry, ioapic_t* apic, uint64_t i, uint64_t tableLength){
    uint64_t len = tableLength - i;
    uint64_t index = sizeof(ioAPICEntry_t);
    while(index < len){
        madtEntryHeader_t* entry = (madtEntryHeader_t*)((uint64_t)ioapicEntry + index);
        if(entry->entryType == IOAPIC_ISO_ENTRY){
            apic->numOverrides++;
        }
        else if(entry->entryType == IOAPIC_NMI_ENTRY){
            apic->numNMI++;
        }
        else {
            return;
        }
        index += entry->length;
    }
}

//TODO: Refactor this
static ioapic_t* ioapicParse(const madtFirstEntry_t* firstEntry, uint32_t numIoapics){
    uint32_t tableLength = firstEntry->madtHeader.length;
    int apicIndex = -1;
    int nmiIndex = -1;
    int overrideIndex = -1;
    ioapic_t* apics = malloc(numIoapics*sizeof(ioapic_t)); 

    uint64_t i = sizeof(madtFirstEntry_t);
    while(i < tableLength){
        madtEntryHeader_t* entry = (madtEntryHeader_t*)((uint64_t)firstEntry + i);
        if(entry->entryType == IOAPIC_ENTRY){
            ++apicIndex;
            nmiIndex = -1;
            overrideIndex = -1;
            ioAPICEntry_t* apicEntry = (ioAPICEntry_t*)entry;
            ioapic_t* apic = apics + apicIndex;
            apic->id = apicEntry->ioAPICId;
            apic->gsiBase = apicEntry->gsiBase;
            
            uint16_t offset = apicEntry->ioAPICAddrs & 0xfff;
            uint64_t vAddrs = (uint64_t)mapPage(apicEntry->ioAPICAddrs, PRESENT | PCD | R_W, 0);
            apic->address = (void*)(vAddrs | offset);

            ioapicGetNumbers(apicEntry, apic, i, tableLength);
            apic->nmis = apic->numNMI > 0 ? malloc(apic->numNMI*sizeof(ioapicNMI_t)) : NULL;
            apic->overrides = apic->numOverrides > 0 ? malloc(apic->numOverrides*sizeof(ioapicOverride_t)) : NULL;
            apic->numEntries = ((ioapicReadReg(apic->address, IOAPIC_VERSION_REGISTER) >> 16) & 0xff) + 1; 

            memset(apic->entries, 0, sizeof(apic->entries));
        }
        else if(entry->entryType == IOAPIC_ISO_ENTRY){
            ++overrideIndex;
            ioAPICisoEntry_t* overrideEntry = (ioAPICisoEntry_t*)entry;
            ioapicOverride_t* override = apics[apicIndex].overrides + overrideIndex;

            override->source = overrideEntry->IRQSource;
            override->inputVector = overrideEntry->gsi - apics[apicIndex].gsiBase;
            override->flags = overrideEntry->flags;
        }
        else if(entry->entryType == IOAPIC_NMI_ENTRY){
            ++nmiIndex;

            ioAPICnmiEntry_t* nmiEntry = (ioAPICnmiEntry_t*)entry;
            ioapicNMI_t* nmi = apics[apicIndex].nmis + nmiIndex;
            nmi->inputVector = nmiEntry->gsi - apics[apicIndex].gsiBase;
            nmi->flags = nmiEntry->flags;
        }
        i += entry->length;
    }

    return apics;
}

ioapicSetup_t* ioapicInit(const SDTHeader_t* madt){
    madtFirstEntry_t* firstEntry = (madtFirstEntry_t*)madt;
    uint32_t tableLength = firstEntry->madtHeader.length;

    ioapicSetup_t* apics = malloc(sizeof(ioapicSetup_t));
    apics->number = 0;

    uint64_t i = sizeof(madtFirstEntry_t);
    while(i < tableLength){
        madtEntryHeader_t* entry = (madtEntryHeader_t*)((uint64_t)firstEntry + i);
        if(entry->entryType == IOAPIC_ENTRY){
            apics->number++;
        }
        i += entry->length;
    }

    apics->ioapics = ioapicParse(firstEntry, apics->number);

    for(uint32_t i = 0; i < apics->number; ++i){
        uint32_t id = apics->ioapics[i].id << 24;
        ioapicWriteReg(apics->ioapics[i].address, IOAPIC_ID_REGISTER, id);
    }

    return apics;
}

static lapic_t* lapicParse(const madtFirstEntry_t* firstEntry, uint8_t* numLapics){
    *numLapics = 0;

    uint32_t tableLen = firstEntry->madtHeader.length;
    uint64_t i = sizeof(madtFirstEntry_t);
    while(i < tableLen){
        madtEntryHeader_t* entry = (madtEntryHeader_t*)(((uint64_t)firstEntry) + i);
        if(entry->entryType == LAPIC_ENTRY){
            ++*numLapics;
        }

        i += entry->length;
    }

    if(*numLapics == 0)
        return NULL;

    lapic_t* lapics = malloc(sizeof(lapic_t) * *numLapics);
    i = sizeof(madtFirstEntry_t);
    uint8_t index = 0;
    while(i < tableLen){
        madtEntryHeader_t* entry = (madtEntryHeader_t*)(((uint64_t)firstEntry) + i);
        if(entry->entryType == LAPIC_ENTRY){
            lapic_t* apic = lapics + index;
            LAPICEntry_t* lapicEntry = (LAPICEntry_t*)entry;

            apic->id = lapicEntry->APICId;
            apic->enabled = lapicEntry->flags & 1;
            apic->onlineCapable = lapicEntry->flags & 2;

            ++index;
        }

        i += entry->length;
    }
    return lapics;
}

lapicSetup_t* lapicInit(const SDTHeader_t* madt){
    madtFirstEntry_t* firstEntry = (madtFirstEntry_t*)madt;
    lapicSetup_t* setup = malloc(sizeof(lapicSetup_t));
    if(!setup){
        return NULL;
    }

    uint64_t pAddrs = firstEntry->LAPICAddrs;
    uint16_t offset = pAddrs & 0xfff;
    uint64_t vAddrs = (uint64_t)mapPage(pAddrs, R_W | PRESENT | PCD, 0);
    void* addrs = (void*)(vAddrs | offset);

    setup->registerAddress = addrs;

    uint32_t ebx, _;
    cpuId(&_, &ebx, &_, &_);
    setup->bootApicId = (ebx >> 24) & 0xff;
    
    setup->lapics = lapicParse(firstEntry, &setup->numLapics);

    enableLAPIC(setup);

    return setup;
}

