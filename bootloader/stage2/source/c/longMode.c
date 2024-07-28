#include "stdlib.h"
#include "display/display.h"
#include "system/interrupts.h"
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
    
    char* test = malloc(2000);
    printf("Malloc complete\n");
    memcpy(test, "hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaajjjjjjjjjjjjjjjjjjjjjjjjjjsdfsfklajkjhjahfjhaljfhajklhfkajhfjkahfjhalfkjhalkjflajkhflkajshflkajhflkjahflkjahlfkjhalfkjhalkjsfhlakjhflkajhfdlkajhflkajhflakjhfljkhafldjhalsjfaljsfhlakjshflkajsflkahsflkjahsflkjahslfjkhalsjkhflajksfhlaksjfljkahsfljkhasflkjhasflkjhalskjhflaskhdflkajshflkahsdflkhalskfjhlaksjhfklajshflkjshfkljahsfdkllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll", 618);
    printf("%s\n", test);
    printf("Address: %p\n", (void*)test);
    printf("Interrupts enabled\n");
    __asm__("int 31");

    halt();
}