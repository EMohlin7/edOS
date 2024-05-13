#include "display.h"
#include "stdlib.h"
#define VID_MEM ((uint16_t*)0x00000000000B8000)
#define NUM_ROWS 25
#define NUM_COLUMNS 80

uint64_t cursorPos = 0;

void printChar(char character, uint8_t color){
    if(character == '\n'){
        uint64_t y = cursorPos / NUM_COLUMNS;
        cursorPos = (y+1)*NUM_COLUMNS;
    }
    else{
        uint16_t c = ((uint16_t)color << 8) | character;
        uint16_t* dest = VID_MEM;
        dest[cursorPos++] = c;
    }
    

    if(cursorPos > NUM_ROWS*NUM_COLUMNS-1)
        scroll(-1);
}

void print(char character){
    printChar(character, WHITE_BLACK_CHAR);
}

void setCursorPos(uint64_t pos){
    cursorPos = pos;
}

void clearScreen(){
    memset(VID_MEM, NULL, NUM_COLUMNS*NUM_ROWS*2);
}

//TODO: Fix so that you can't scroll to far. Also implement scrolling up.
void scroll(int lines){


    if(lines > 0)
        return;

    //Start at lines to not scroll to far up
    for(int i = lines; i < NUM_ROWS; ++i){
        uint16_t* src = VID_MEM + NUM_COLUMNS*i;
        uint16_t* dest = src + NUM_COLUMNS*lines;
        //Move line
        memcpy(dest, src, NUM_COLUMNS+2);

        //Clear line
        memset(src, NULL, NUM_COLUMNS*2);
    }
    setCursorPos(cursorPos + NUM_COLUMNS*lines);
    lines = abs(lines);
}