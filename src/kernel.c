#include "common/common.h"
#include "kernel/uart.h"
#include "kernel/rtc.h"
#include "lib/string.h"

void kmain(uint32_t dtb_pointer) {
    uint32_t epoch = rtc_get_epoch();

    uart_printk("Welcome to the AArch64 Kernel\n\n");
    uart_printk("Kernel loaded at (epoch): %d\n", (int)epoch);

    while (1);  // Halt
}
