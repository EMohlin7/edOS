#include <stdint.h>
#define NUM_ENTRIES 512

typedef struct
{
    uint64_t entries[512];
} PT_t;


static uint16_t PML4Entries = 1, PML4CurrIndex = 0;
static uint16_t PDPEntries = 1, PDPCurrIndex = 0;
static uint16_t PDEntries = 1, PDCurrIndex = 0;
static uint16_t PTEntries = 512, PTCurrIndex = 0;

static enum PTLevel{
    PML4,
    PDP,
    PD,
    PT
};

void* getPT(uint32_t index){
    uint64_t adrs = 0xFFFFFF8000000000;
    uint64_t PML4Offset = index / (512*512);
    uint64_t PDPOffset = (index / 512) % 512;
    uint64_t PDOFFset = index % 512;

    adrs += PML4Offset << 30;
    adrs += PDPOffset << 21;
    adrs += PDOFFset << 12;

    //Set 12 last bits to zero (Page offset)
    //adrs &= ~0xFFF;

    return (PT_t*)adrs;
}


void* getPD(uint32_t index){
    uint64_t adrs = 0xFFFFFFFFC0000000;
    uint64_t PML4Offset = index / 512;
    uint64_t PDPOffset = index % 512;

    adrs += PML4Offset << 21;
    adrs += PDPOffset << 12;

    return (PT_t*)adrs;
}

void* getPDP(uint16_t index){
    uint64_t adrs = 0xFFFFFFFFFFE00000;
    uint64_t PML4Offset = index % 512;

    adrs += PML4Offset << 12;

    return (PT_t*)adrs;
}

void* getPML4(){
    return (PT_t*)~0xFFF;
}


void* mapPage(uint64_t physAddress, uint64_t numPages, uint16_t flags){
    flags &= 0xFFF;
    uint64_t address = (physAddress & ~0xFFF) | flags;  

    if(PML4Entries == NUM_ENTRIES)
        return NULL;
    
    if(PDPEntries == NUM_ENTRIES)
        ;
    
    if(PDEntries == NUM_ENTRIES)
        ;
    
    if(PTEntries == NUM_ENTRIES)
        ;
}