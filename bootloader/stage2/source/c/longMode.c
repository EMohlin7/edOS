#include "stdlib.h"
#include "display.h"
#include "interrupts.h"
#include "sysInfo.h"
#include "vmm.h"

typedef struct 
{
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t attributes; //Only for ACPI 3.0+
} MemRegion;

typedef struct MemNode
{
    struct MemNode* next;
    struct MemNode* prev;
    uint64_t base;
    uint64_t length;
} MemNode_t;

uint32_t test;

void* getPT(uint32_t index);
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