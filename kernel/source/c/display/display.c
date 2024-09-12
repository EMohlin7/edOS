#include "display/display.h"
#include "stdlib.h"
#include "memMgmt/vmm.h"
#define VID_PHYS_ADRS (0x00000000000B8000)
#define NUM_ROWS 25
#define NUM_COLUMNS 80

static uint64_t cursorPos = 0;
static uint16_t* vidMemory;

//TODO: Fix cursor

void printChar(char character, uint8_t color){
    if(character == '\n'){ //New line
        uint64_t y = cursorPos / NUM_COLUMNS;
        cursorPos = (y+1)*NUM_COLUMNS;
    }
    else{
        uint16_t c = ((uint16_t)color << 8) | character;
        vidMemory[cursorPos++] = c;
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
    memset(vidMemory, NULL, NUM_COLUMNS*NUM_ROWS*2);
}

//TODO: Fix so that you can't scroll to far. Also implement scrolling up.
void scroll(int lines){


    if(lines > 0)
        return;

    //Start at lines to not scroll to far up
    for(int i = lines; i < NUM_ROWS; ++i){
        uint16_t* src = vidMemory + NUM_COLUMNS*i;
        uint16_t* dest = src + NUM_COLUMNS*lines;
        //Move line
        memcpy(dest, src, NUM_COLUMNS*2);

        //Clear line
        memset(src, NULL, NUM_COLUMNS*2);
    }
    setCursorPos(cursorPos + NUM_COLUMNS*lines);
    lines = abs(lines);
}

void initDisplay(void){
    vidMemory = mapPage(VID_PHYS_ADRS, PRESENT | PWT | R_W, 0);
}
