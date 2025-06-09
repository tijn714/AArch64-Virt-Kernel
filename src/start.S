.section .text
.global _start

_start:
    // Set up the stack
    ldr x0, =_stack_top
    mov sp, x0

    // Save the DTB pointer from x0 (passed from QEMU) into w0 (32-bit)
    mov w1, w0         // w1 = DTB pointer

    // Check if DTB pointer is zero
    cbz w1, .halt

    // Enable floating point/SIMD
    mrs x0, CPACR_EL1
    orr x0, x0, #(3 << 20)     // CPACR_EL1: set FPEN bits (20-21) to 0b11
    msr CPACR_EL1, x0
    isb                        // Synchronize

    // Zero FP status/control
    mov x0, #0
    msr FPCR, x0
    msr FPSR, x0

    // Call kmain(uint32_t dtb_pointer)
    mov w0, w1
    bl kmain

.halt:
    // Infinite halt loop
1:  wfe
    b 1b

// Stack section
.section .bss
.align 12
.space 4096
_stack_top:
