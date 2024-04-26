#include <stdint.h>
#include "printf.h"
#include "display.h"


__attribute__((noreturn))
void longMode(){
    clearScreen();
    printf("Long mode enabled");
    while(1);
}