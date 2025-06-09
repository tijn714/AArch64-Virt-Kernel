#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#ifndef NULL
  #define NULL ((void*)0UL)
#endif


#ifndef AARCH64OS_
  #define AARCH64OS_
#endif

typedef unsigned int size_t;

typedef struct {
  bool Initialized;
  uint8_t uart_base;
  uint32_t ram_size;
  uint32_t ram_start;
  uint32_t cpu_frequency;
} device_tree_struct;

static inline void wait_cycles(uint32_t cycles) {
    for (uint32_t temp = 0; temp != cycles; temp++) {
        __asm__ volatile("nop");
    }
}

#endif // COMMON_H
