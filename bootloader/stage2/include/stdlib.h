#ifndef STDLIB_H
#define STDLIB_H
#include <stdint.h>

typedef _Bool bool;
#define true 1
#define false 0


#define halt() __asm__("hlt")

/// @brief Copies one array to another
/// @param dest Destination array
/// @param src Source array
/// @param n The number of bytes to be copied
static inline void memcpy(void* dest, void* src, uint64_t n){
    __asm__("cld\n\t rep movsb \n\t" : /*No output*/ : "D" (dest), "S" (src), "c" (n));
}

/// @brief Copies the same byte n times to the array pointed to by dest
static inline void memset(void* dest, char val, uint64_t n){
    __asm__("cld\n\t rep stosb \n\t" : /*No output*/ : "a" (val), "D" (dest), "c" (n));
}

/// @brief Compares if two blocks of memory are equal
/// @param ptr1 Pointer two the first block
/// @param ptr2 Pointer to the second block
/// @param n Number of bytes to compare
/// @return 0 if the blocks are equal. byte1 - byte2 of the first byte that don't match
static inline int memcmp(void* ptr1, void* ptr2, uint64_t n){
    char diff = 0;
    __asm__("cld                                    \n\t"\
            "repe cmpsb                             \n\t"\
            "dec rdi \n\t"                               \
            "dec rsi \n\t"                               \
            "mov %0, byte [rdi]                     \n\t"\
            "sub %0, byte [rsi]                     \n\t"\
            : "=r"(diff)                                 \
            : "D"(ptr1), "S"(ptr2), "c"(n-1)
    );

    return diff;
}

__attribute__((format(printf,1,2)))
uint64_t printf(const char* format, ...);

static inline uint64_t abs(int64_t val){
    return val < 0 ? -val : val;
}

#endif //STDLIB_H