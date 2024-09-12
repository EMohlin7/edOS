#include "stdlib.h"

void memdump(const void* start, size_t length, uint8_t bpg, uint8_t gpl){
    printf("|------memdump------|\n");
    if(bpg <= 0)
        bpg = 1;
    uint64_t groups = 0;
    printf("%#010lx ", (uint64_t)start);

    uint64_t i = 0;
    for(; i < length;){        
        for(uint64_t j = i; j < MIN(length, i+bpg); ++j){
            printf("%02x", *((uint8_t*)start+j));
        }
        i+=MIN(length-i, bpg);
        printf(" ");
        ++groups;
        if(groups % gpl == 0)
            printf("\n%#010lx ", (uint64_t)((uint8_t*)start+i));
    }
    printf("\n%#010lx ", (uint64_t)((uint8_t*)start+i));
    printf("\n|------memdump------|\n");
}