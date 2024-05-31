#ifndef BIT32_H
#define BIT32_H
#include <stdint.h>
#ifdef BIT32
    #define multibit(x) bit32##x 
    #define bitshared extern
    #define ptoi (uint32_t)
    typedef uint32_t pint;
#else
    #define multibit(x) x
    #define bitshared
    #define ptoi (uint64_t)
    typedef uint64_t pint;
#endif



#endif //BIT32_H