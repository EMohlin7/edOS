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
extern void memcpy(void* dest, void* src, uint64_t n);

/// @brief Copies the same byte n times to the array pointed to by dest
extern void memset(void* dest, uint8_t val, uint64_t n);

/// @brief Compares if two blocks of memory are equal
/// @param ptr1 Pointer to the first block
/// @param ptr2 Pointer to the second block
/// @param n Number of bytes to compare
/// @return 0 if the blocks are equal. byte1 - byte2 of the first byte that don't match
extern int memcmp(void* ptr1, void* ptr2, uint64_t n);

__attribute__((format(printf,1,2)))
uint64_t printf(const char* format, ...);

void* malloc(uint64_t size);

static inline uint64_t abs(int64_t val){
    return val < 0 ? -val : val;
}

#endif //STDLIB_H