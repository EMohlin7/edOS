#include "display.h"
#define VID_MEM (uint16_t*)0x00000000000B8000

uint64_t cursorPos = 0;

void printChar(char character, uint8_t color){
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

}