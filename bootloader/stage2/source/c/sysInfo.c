#include "sysInfo.h"
#include "stdlib.h"
void printSysInfo(void){
    char vendor[13];
    vendor[12] = NULL;
    cpuId(0, (uint32_t*)vendor, (uint32_t*)(vendor+4), (uint32_t*)(vendor+8));
    printf("CPU vendor: %s\n", vendor);
}

