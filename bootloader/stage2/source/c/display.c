#include "display.h"
#define VID_MEM ((uint16_t*)0x00000000000B8000)
#define NUM_ROWS 25
#define NUM_COLUMNS 80

uint64_t cursorPos = 0;

void printChar(char character, uint8_t color){
    if(character == '\n'){
        uint64_t y = cursorPos / NUM_COLUMNS;
        cursorPos = (y+1)*NUM_COLUMNS;
        return;
    }
    
    uint16_t c = ((uint16_t)color << 8) | character;
    uint16_t* dest = VID_MEM;
    dest[cursorPos++] = c;
}

void print(char character){
    printChar(character, WHITE_BLACK_CHAR);
}

void setCursorPos(uint64_t pos){
    cursorPos = pos;
}

void clearScreen(){
    for(int i = 0; i < NUM_COLUMNS*NUM_ROWS; ++i){
        uint16_t* mem = VID_MEM;
        mem[i] = NULL;
    }
}