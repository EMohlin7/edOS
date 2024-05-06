#include "stdlib.h"
#include "printf.h"
#include "display.h"
#include "interrupts.h"


void longMode(){
    clearScreen();
    printf("Long mode enabled\n");
    
    initIDT();
    printf("Interrupts enabled\n");
    halt();
}