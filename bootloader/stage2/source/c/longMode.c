#include <stdint.h>
#include "printf.h"
#define VID_MEM (short*)0xB8000


__attribute__((noreturn))
void longMode(){
    printf("Long mode enabled");
    while(1);
}