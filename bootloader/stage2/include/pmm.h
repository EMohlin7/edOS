#ifndef PMM_H
#define PMM_H
#include <stdint.h>
#include "bit32.h"
#include "memory.h"

#ifdef BIT32
#endif
void initPMM(void);

uint64_t multibit( allocatePhysPage(uint64_t physAdrs));

#endif //PMM_H