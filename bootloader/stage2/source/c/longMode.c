#include "stdlib.h"
#include "display.h"
#include "interrupts.h"
#include "sysInfo.h"

void longMode(){
    clearScreen();
    printf("Long mode enabled\n");
    printSysInfo();
    initInterrupts();
    printf("Interrupts enabled\n");

    __asm__("int 31");

    halt();
}