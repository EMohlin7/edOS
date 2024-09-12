#include "memMgmt/vmm.h"
#include "stdlib.h"
#include "memMgmt/pmm.h"

typedef struct
{
    uint64_t entries[512];
} PT_t;

typedef struct 
{
    uint32_t entries;
    uint32_t currIndex;
} PTData_t;


PTData_t tableData[4];

typedef enum {
    PML4Index = 0,
    PDPIndex = 1,
    PDIndex = 2,
    PTIndex = 3
} PTIndex_t;

#define invalidateTLB(addrs) __asm__("invlpg [%0]" : : "r"((uint64_t)(addrs))); 

//#ifdef BIT32


//#else //#ifdef BIT32

//Get the virtual address of the Page table with index 'index'
static PT_t* getPT(uint32_t index){
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

//Get the virtual address of the Page descriptor table with index 'index'
static PT_t* getPD(uint32_t index){
    uint64_t adrs = 0xFFFFFFFFC0000000;
    uint64_t PML4Offset = index / 512;
    uint64_t PDPOffset = index % 512;

    adrs += PML4Offset << 21;
    adrs += PDPOffset << 12;

    return (PT_t*)adrs;
}

//Get the virtual address of the Page descriptor pointer table with index 'index'
static PT_t* getPDP(uint16_t index){
    uint64_t adrs = 0xFFFFFFFFFFE00000;
    uint64_t PML4Offset = index % 512;

    adrs += PML4Offset << 12;

    return (PT_t*)adrs;
}

//Get the virtual address of the Page map level 4 table
static PT_t* getPML4(){
    return (PT_t*)~0xFFF;
}

void initVMM(uint64_t programSize){
    initPMM();
    
    //Make the first entry of the tables point to the next table
    for(int i = 0; i < 3; ++i){
        tableData[i].entries = 1;
    }

    //Map physical address 0x0 to end of program
    PT_t* pt = getPT(0);
    uint32_t numPages = NUM_PT_ENTRIES;//(PROGRAM_POS + programSize)/PAGE_SIZE + 2;
    for(uint32_t i = 0; i < numPages; ++i){
        allocatePhysPage(pt->entries[i]);
        tableData[PTIndex].entries += 1;
    }
}

/// @brief Entry to table. Convert address of a table entry to the address of the pointed to table.
/// @param entry The virtual address of the entry using recursive mapping.
/// @return The pointer to the beginning of the table pointed to by the entry. If entry is an entry in a page table, the pointer is the virtual address of the page mapped in that entry.
static void* etot(uint64_t entry){
    uint64_t offset = entry & 0xFFF;
    offset /= 8;

    
    entry &= ~0xFFF;       //Set PT offset and physical offset to zero. (Least significant 21 bits)
    entry <<= 9;              //Because of recusive mapping the tables will be offset in the address
    entry |= (offset << 12);

    //Calculate the correct sign extension
    if(entry & (1ul << 47))
        entry |= 0xFFFF000000000000;
    else
        entry &= 0x0000FFFFFFFFFFFF;

    return (void*)entry;
}


/// @brief Create an entry in a table.
/// @param table Pointer to the table in which to add the entry.
/// @param pAdrs The physical address that the entry will point to. This value will always be page aligned.
/// @param index The index in the table where to create the entry.
/// @param flags The flags of the entry.
/// @param nx The no execute bit. 1 means that this memory region cannot be executed.
/// @return A pointer to the newly created entry in the table.
static void* mapEntry(PT_t* table, uint64_t pAdrs, uint16_t index, uint64_t flags, uint64_t nx){
    uint64_t entry = 0;
    pAdrs &= ~0xFFF;    //Make sure the address is page aligned
    nx &= 1;            //1 bit
    flags &= 0xFFF;     //12 bit
    index &= 0x1FF;     //9 bit

    entry += (nx << 63) | pAdrs | flags;
    table->entries[index] = entry;

    return table->entries + index;
}


static void* addEntry(PTIndex_t index, uint64_t pAdrs, uint16_t flags, uint8_t nx){
    PTData_t* data = tableData + index;
    PT_t* table;
    
    if(data->entries == NUM_PT_ENTRIES){
        if(index != PML4Index){
            data->entries = 0;
            data->currIndex += 1;
            void* new = etot((uint64_t)addEntry(index-1, NULL, flags, nx));
            memset(new, 0, sizeof(PT_t));   // Clear the new table so it is filled with zeros
        }
        else if(data->entries >= NUM_PT_ENTRIES-1){   //Last entry is taken for recursive mapping
            return NULL;    //If all PML4 entries are filled the whole virtual memory space is used
        }
    }

    switch (index)
    {
        case PML4Index:
            table = getPML4();
            break;
        case PDPIndex:
            table = getPDP(data->currIndex);
            break;
        case PDIndex:
            table = getPD(data->currIndex);
            break;
        case PTIndex:
            table = getPT(data->currIndex);
            break;
        default:
            __asm__("hlt");
            break;
    }

    pAdrs = allocatePhysPage(pAdrs); 

    void* adrs = mapEntry(table, pAdrs, data->entries, flags, nx);
    data->entries += 1;

    return adrs;
}


void* mapPage(uint64_t physAddress, uint16_t flags, uint8_t nx){
    
    void* entryAdrs = addEntry(PTIndex, physAddress, flags, nx);
    void* vAdrs = etot((uint64_t) entryAdrs);

    return vAdrs;
}

void umapPage(void* addrs){
    uint64_t a = (uint64_t)addrs;
    uint32_t index = 0; //PT index

    a &= 0xfffffffff000; //Remove sign extension and physical offset
    uint64_t ptOffset = (a >> 12) & 0x1ff;
    uint64_t pdOffset = (a >> 21) & 0x1ff;
    uint64_t pdpOffset = (a >> 30) & 0x1ff;
    uint64_t PML4Offset = (a >> 39) & 0x1ff;

    index = pdOffset + pdpOffset*512 + PML4Offset*512*512;
    PT_t* pt = getPT(index);
    uint64_t* ptEntry = &pt->entries[ptOffset];

    //if(*ptEntry == 0)
      //  return;

    *ptEntry = 0; //Clear page table entry;

    tableData[PTIndex].entries -= 1;
    invalidateTLB(addrs);
}

//#endif //BIT32



