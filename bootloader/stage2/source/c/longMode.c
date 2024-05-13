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
    printf("Memcmp test: %d\n", memcmp("Hejsan test", "Hejsan test", 11));
    halt();
}