#ifndef MM_H
#define MM_H

#include <common/common.h>
#include <kernel/dtb.h>

unsigned long get_kernel_size(void);

void mm_init(dtb_ctx *dtb_ctx);
void *kmalloc(size_t size);
void kfree(void *ptr);

#endif // MM_H