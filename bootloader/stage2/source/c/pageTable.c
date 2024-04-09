#include <stdint.h>
#define PAGE_SIZE 0x1000
#define VID_MEM (short*)0xB8000

/// @brief Setup the initial page tables
/// @param PML4 The physical address to place PML4 at. The rest of the tables will be placed directly following this address.
void setupPageTable(uint32_t PML4){
    for(int i = 0; i< length; ++i){
        //mem[i] = data[i];
    }
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
        uint32_t* currTable = (uint32_t*)(tables + i*PAGE_SIZE);
        *currTable = (uint32_t)currTable + PAGE_SIZE + 3;     //Add 3 to set P and R/W bits to 1
    }


    //Map all of the entries in the pt to physical memory. First two MiB 0x0 - 0x200000
    uint64_t* pt = (uint64_t*)PT;
    for(int i = 0; i < 512; ++i){
        pt[i] = PAGE_SIZE*i + 3;
    }
}