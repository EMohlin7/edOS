#ifndef STDLIB_H
#define STDLIB_H
#include "definitions.h"

#define halt() __asm__("hlt")

#define abs(val) (val < 0 ? -val : val)
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

/// @brief Get the size of a struct member
/// @param structType The struct type containing the member
/// @param member Name of the member which size is desired
/// @returns The size in size_t of the member "member" in the struct "structType"
#define sizeofMember(structType, member) (sizeof(((structType*)0)->member))



/// @brief Copies one array to another
/// @param dest Destination array
/// @param src Source array
/// @param n The number of bytes to be copied
extern void memcpy(void* dest, const void* src, uint64_t n);

/// @brief Copies the same byte n times to the array pointed to by dest
extern void memset(void* dest, uint8_t val, uint64_t n);

/// @brief Compares if two blocks of memory are equal
/// @param ptr1 Pointer to the first block
/// @param ptr2 Pointer to the second block
/// @param n Number of bytes to compare
/// @return 0 if the blocks are equal. byte1 - byte2 of the first byte that don't match
extern int memcmp(const void* ptr1, const void* ptr2, uint64_t n);

__attribute__((format(printf,1,2)))
uint64_t printf(const char* format, ...);

void* malloc(uint64_t size);

void memdump(const void* start, size_t length, uint8_t bpg, uint8_t gpl);

///@brief Make a constant sized string NULL terminated
///@param out The name of the new NULL terminated string. It will be sizeof("string")+1 big 
///@param string The constant sized array that will be copied to "out"
#define makeNullTerminated(out, string)  \
    char out[sizeof(string)+1];               \
    out[sizeof(string)] = NULL;               \
    memcpy(out, string, sizeof(string));      \
    


#endif //STDLIB_H