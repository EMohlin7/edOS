#include <stdint.h>
#include <stdarg.h>
#define VID_MEM (uint16_t*)0x00000000000B8000


static void print(char character, uint8_t color, uint64_t index){
    uint16_t c = ((uint16_t)color << 8) | character;
    uint16_t* dest = VID_MEM;
    dest[index] = c;
}

uint64_t printf(const char* format){
    uint64_t i = 0;
    while (format[i] != 0)
    {
        print(format[i], 0x0F, i);
        ++i;
    }
    
    return i-1;
}