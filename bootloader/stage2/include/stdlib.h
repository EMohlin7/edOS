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
#define memcpy(dest, src, n)                                \
    __asm__("mov rdi, %0\n\t"                               \
            "mov rsi, %1\n\t"                               \
            "mov rcx, %2\n\t"                               \
            "rep movsb \n\t"                                \
            : /*No output*/                                 \
            : "rmi" (dest), "rmi" (src), "rmi" (n)          \
            : "rdi", "rsi", "rcx"                           \
        )

/// @brief Copies the same byte n times to the array pointed to by dest
#define memcpyConst(dest, byte, n)                          \
    __asm__("mov al, %0\n\t"                                \
            "mov rdi, %1\n\t"                               \
            "mov rcx, %2\n\t"                               \
            "rep stosb\n\t"                                 \
            : /*No output*/                                 \
            : "rmi" (byte), "rmi" (dest), "rmi" (n)         \
            : "al", "rdi", "rcx"                            \
        )


static inline uint64_t abs(int64_t val){
    return val < 0 ? -val : val;
}

#endif //STDLIB_H