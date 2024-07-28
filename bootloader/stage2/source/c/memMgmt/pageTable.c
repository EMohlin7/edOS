#include <stdint.h>
#include "memMgmt/vmm.h"
//This code is compiled as 32 bit code because it is used to setup the page tables before entering long mode

/// @brief Setup the initial page tables
/// @param PML4 The physical address to place PML4 at. The rest of the tables will be placed directly following this address.
void setupPageTable(uint32_t PML4, uint32_t programSize){
    uint32_t PDP = PML4 + PAGE_SIZE;
    uint32_t PD = PDP + PAGE_SIZE;
    uint32_t PT = PD + PAGE_SIZE;

    uint8_t* tables = (uint8_t*)PML4;
    //Initialize all tables to 0
    for(uint32_t i = 0; i < PT + PAGE_SIZE - PML4; ++i){
        tables[i] = 0;
    }

    //Make the first entry of the tables point to the next table
    for(int i = 0; i < 3; ++i){
        uint64_t* currTable = (uint64_t*)(tables + i*PAGE_SIZE);
        *currTable = (uint32_t)currTable + PAGE_SIZE + 3;     //Add 3 to set P and R/W bits to 1
    }

    //Point last entry of PML4 to itself to allow for recursive mapping
    uint64_t* tmp = (uint64_t*)PML4;
    tmp[511] = PML4 + 3;


    //Map all of the entries in the pt to physical memory. First two MiB 0x0 - 0x200000-1
    uint64_t* pt = (uint64_t*)PT;
    uint32_t numPages = NUM_PT_ENTRIES;//(PROGRAM_POS + programSize)/PAGE_SIZE + 2;
    for(uint32_t i = 0; i < numPages; ++i){
        pt[i] = PAGE_SIZE*i + 3;
    }
}