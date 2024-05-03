#include <stdint.h>
#ifndef DISPLAY_H
#define DISPLAY_H

#define WHITE_BLACK_CHAR 0x0F

void printChar(char character, uint8_t color);
void print(char character);

void setCursorPos(uint64_t pos);

void clearScreen();

#endif //DISPLAY_H