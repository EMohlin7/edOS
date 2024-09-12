#include "stdlib.h"
#include "display/display.h"
#include "system/interrupts/interrupts.h"
#include "system/sysInfo.h"
#include "memMgmt/vmm.h"

void longMode(uint64_t programSize){
    initVMM(programSize);
    initDisplay();
    clearScreen();
    printf("Long mode enabled\n");
    printf("Program size: %lu\n", programSize);
    
    printSysInfo();

    initInterrupts();
    
    printf("Interrupts enabled\n");

    while(true)
        halt();
}