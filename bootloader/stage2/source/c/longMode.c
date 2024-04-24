#include <stdint.h>
#include "printf.h"
#define VID_MEM (short*)0xB8000


__attribute__((noreturn))
void longMode(){
    

    //char string[] = \nHej";

    printf(" %ld", printf("Hejsan!%#.4x", 3500));
    while(1);
}