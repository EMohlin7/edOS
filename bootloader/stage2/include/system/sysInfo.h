#ifndef SYSINFO_H
#define SYSINFO_H
#include "stdint.h"


/// @brief Calls the "cpuid" x86 instruction
/// @param eax uint32 value that will be placed in eax and decide what info will be returned. Also holds returned eax value
/// @param ebx Pointer to uint32 that will hold the returned ebx value
/// @param edx Pointer to uint32 that will hold the returned edx value
/// @param ecx Pointer to uint32 that will hold the returned ecx value
static inline void cpuId(uint32_t* eax, uint32_t* ebx, uint32_t* edx, uint32_t* ecx){
    __asm__("cpuid \n\t"                              \
            : "=a" (*eax), "=b" (*ebx), "=d" (*edx), "=c" (*ecx)   \
            : "a" (*eax)
    );
}


void printSysInfo(void);

#endif //SYSINFO_H