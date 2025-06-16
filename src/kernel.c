#include <common/common.h>
#include <kernel/uart.h>
#include <kernel/rtc.h>
#include <kernel/dtb.h>
#include <lib/string.h>

#ifndef REQURED_COMMON_H
 #error "The common/common.h is required to include to compile a driver."
#endif

// External reference to DTB pointer set in start.S
extern uint64_t _dtb_start;

void kmain(void) {
    uart_printk("Welcome to the QEMU-VIRT Kernel\n\n");

    #ifdef DEBUG_ENABLED
        uart_printk("[INFO] DEBUG MODE IS ENABLED! PLEASE NOTE THIS IS NOT RECOMMENDED FOR PRODUCTION\n");
    #endif

    // Initialize DTB context
    dtb_context_t dtb_ctx;
    memset(&dtb_ctx, 0, sizeof(dtb_context_t));

    // Get DTB pointer
    void *dtb_blob = (void *)_dtb_start;
    if (dtb_blob == NULL) {
        uart_printk("[ERROR] No DTB provided, halting...\n");
    } else {
        // Parse DTB
        dtb_parse(dtb_blob, &dtb_ctx);
        uart_printk("[INFO] DTB parsing completed, found %u devices\n", dtb_ctx.device_count);

        // Print parsed devices
        dtb_print_devices(&dtb_ctx);
    }

    while (1);  // Halt
}