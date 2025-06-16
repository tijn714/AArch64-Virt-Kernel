#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#ifndef NULL
  #define NULL ((void*)0UL)
#endif


#ifndef REQURED_COMMON_H
  #define REQURED_COMMON_H
#endif

typedef unsigned int size_t;

static inline void wait_cycles(uint32_t cycles) {
    for (uint32_t temp = 0; temp != cycles; temp++) {
        __asm__ volatile("nop");
    }
}

#endif // COMMON_H
