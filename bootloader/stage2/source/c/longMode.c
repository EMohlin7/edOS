#include <stdint.h>
#define VID_MEM (short*)0xB8000

uint64_t printf(const char* format);

__attribute__((noreturn))
void longMode(){
    

    //char string[] = \nHej";

    printf("Hejasna balooogasd!");
    while(1);
}