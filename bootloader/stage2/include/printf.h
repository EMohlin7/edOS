#include <stdint.h>
#ifndef PRINTF_H
#define PRINTF_H

__attribute__((format(printf,1,2)))
uint64_t printf(const char* format, ...);

#endif //PRINTF_H