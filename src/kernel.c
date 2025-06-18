#include <common/common.h>
#include <kernel/uart.h>
#include <kernel/rtc.h>
#include <kernel/dtb.h>
#include <kernel/mm.h>
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
    dtb_ctx dtb_ctx;
    memset(&dtb_ctx, 0, sizeof(dtb_ctx));

    // Get DTB pointer
    void *dtb_blob = (void *)_dtb_start;
    if (dtb_blob == NULL) {
        uart_printk("[ERROR] No DTB provided, halting...\n");
    }

    // Parse DTB
    dtb_parse(dtb_blob, &dtb_ctx);
    uart_printk("[INFO] DTB parsing completed, found %u devices\n", dtb_ctx.device_count);

    // Print parsed devices
    #ifdef DEBUG_ENABLED
        uart_printk("[DEBUG] DTB devices:\n");
        dtb_print_devices(&dtb_ctx);
        uart_printk("=========================================================\n");
    #endif


    const device_t *mem_dev = NULL;
    bool memory_found = false;

    for (size_t i = 0; i < dtb_ctx.device_count; i++) {
        const device_t *dev = &dtb_ctx.devices[i];
        if (dev->is_memory) {
            uart_printk("RAM Detected: %s \nREGS: ", dev->name);
            for (size_t j = 0; j < dev->regs_len; j++) {
                uart_printk("%02x", dev->regs[j]);
            }
            uart_printk("\n");
            memory_found = true;
            mem_dev = dev;  // Save the memory device
            break;  // We only need one memory device
        }  
    }
    if (!memory_found) {
        uart_printk("[ERROR] No memory device found, make sure you run the kernel with a valid DTB. and at least 2GB of RAM\n");
        while (true) asm("nop");
    }
    unsigned long kernel_size = get_kernel_size();
    uart_printk("[INFO] Kernel size: %lu bytes\n", kernel_size);


    if (!mem_dev || mem_dev->regs_len < 8) {
        uart_printk("[ERROR] Invalid memory device or insufficient regs length\n");
        while (1);  // Halt
    }

    while (1);  // Halt
}