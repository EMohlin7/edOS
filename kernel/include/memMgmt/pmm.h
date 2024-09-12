#ifndef PMM_H
#define PMM_H
#include <stdint.h>

#define PAGE_SIZE 0x1000


void initPMM(void);

uint64_t allocatePhysPage(uint64_t physAdrs);

#endif //PMM_H