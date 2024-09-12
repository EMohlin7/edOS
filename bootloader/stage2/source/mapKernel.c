#include <stdint.h>

#define MEMMAP_LENGTH 50
#define NULL 0
#define PAGE_SIZE 0x1000

typedef struct   
{
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t attributes;
} memBlock_t;

extern memBlock_t memoryMap[MEMMAP_LENGTH];


//Maps the kernel into 0xffffff0000000000
uint64_t mapKernel(uint64_t* PML4, uint32_t kernelSize, uint32_t kernelPhysicalAddrs){
    uint64_t addrs = NULL;
    uint32_t numPages = kernelSize / PAGE_SIZE + 1;
    uint32_t numPt = numPages / 512 + 1;
    uint32_t numPd = numPt / 512 + 1;
    uint32_t numPdp = numPd / 512 + 1; 
    uint32_t numTables = numPt + numPd + numPdp;

    
    //Find memBlock that fits 3 page tables
    for(uint32_t i = 0; i < MEMMAP_LENGTH; ++i){
        if(memoryMap[i].base > kernelPhysicalAddrs + kernelSize){
            if(memoryMap[i].type == 1 && memoryMap[i].length > numTables*PAGE_SIZE){
                addrs = memoryMap[i].base + (0x1000 - (memoryMap[i].base & 0xfff));//Make sure its page aligned
                memoryMap[i].length -= addrs + numTables*PAGE_SIZE - memoryMap[i].base;
                memoryMap[i].base = addrs + numTables*PAGE_SIZE;
                break;
            }
        }
    }

    if(addrs == NULL)
        return NULL;

    //Initilialize the tables to 0
    for(uint32_t i = 0; i < numTables*PAGE_SIZE/8; ++i){
        ((uint64_t*)addrs)[i] = 0;
    }

    uint64_t* pdps = (uint64_t*)addrs;
    uint64_t* pds = (uint64_t*)((uint8_t*)pdps + numPdp*PAGE_SIZE);
    uint64_t* pts = (uint64_t*)((uint8_t*)pds + numPd*PAGE_SIZE);

    //Map pdps into the pml4
    for(uint16_t pdp = 0; pdp < numPdp; ++pdp){
        PML4[511-numPdp + pdp] = (((uint64_t)pdps) + PAGE_SIZE*pdp) | 3;
    }

    //Map pds into the pdps
    for(uint32_t pd = 0; pd < numPd; ++pd){
        uint64_t* pdp = (uint64_t*)(((uint64_t)pdps) + pd/512 * PAGE_SIZE);

        pdp[pd % 512] = (((uint64_t)pds) + pd*PAGE_SIZE) | 3;
    }
    
    //Map pts into the pds
    for(uint32_t pt = 0; pt < numPt; ++pt){
        uint64_t* pd = (uint64_t*)(((uint64_t)pds) + pt/512 * PAGE_SIZE);

        pd[pt % 512] = (((uint64_t)pts) + pt*PAGE_SIZE) | 3;
    }

    //Map physical frames into the pts
    for(uint32_t page = 0; page < numPages; ++page){
        uint64_t* pt = (uint64_t*)(((uint64_t)pts) + page/512 * PAGE_SIZE);

        pt[page % 512] = (kernelPhysicalAddrs + page*PAGE_SIZE) | 3;
    }

    return addrs;
}