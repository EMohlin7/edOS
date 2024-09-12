#include "definitions.h"

static inline uint64_t readMSR(uint32_t reg){
    uint32_t edx, eax;
    __asm__("rdmsr" : "=edx"(edx), "=eax"(eax) : "c"(reg));
    
    return (uint64_t)edx<<32 | eax;
}

static inline void writeMSR(uint32_t reg, uint64_t val){
    __asm__("wrmsr" : : "c"(reg), "d"(val>>32), "a"(val&0xFFFFFFFF));
}