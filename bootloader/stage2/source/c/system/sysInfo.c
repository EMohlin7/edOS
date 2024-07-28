#include "system/sysInfo.h"
#include "stdlib.h"
void printSysInfo(void){
    char vendor[13];
    vendor[12] = NULL;
    uint32_t eax = 0;
    cpuId(&eax, (uint32_t*)vendor, (uint32_t*)(vendor+4), (uint32_t*)(vendor+8));
    printf("CPU vendor: %s\n", vendor);
}

