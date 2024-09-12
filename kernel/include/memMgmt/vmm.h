#ifndef VMM_H
#define VMM_H
#include "memMgmt/pmm.h"
#include <stdint.h>

#define PRESENT 1
#define R_W 2           //Read write
#define U_S (1 << 2)    //User/Supervisor
#define PWT (1 << 3)    //Write through caching
#define PCD (1 << 4)    //Cache disable

#define NUM_PT_ENTRIES 512

/// @brief Map a physical memory page into virtual memory.
/// @param physAddress The physical address that will be mapped. This value will always be page aligned.
/// @param numPages The number of pages to map. These will be mapped continiously in virtual memory
/// @param flags The flags of the entry.
/// @param nx The no execute bit. 1 means that this memory region cannot be executed.
/// @return A virtual pointer to the start of the mapped pages. NULL if the address could not be mapped or that number of pages could not be mapped continously.
void* mapPage(uint64_t physAddress, uint16_t flags, uint8_t nx);

void umapPage(void* addrs);

void initVMM(uint64_t programSize);

#endif //VMM_H