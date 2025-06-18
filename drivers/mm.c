#include <common/common.h>
#include <kernel/dtb.h>

extern char _start;
extern char _end;

unsigned long get_kernel_size(void) {
    return (unsigned long)(&_end - &_start);
}