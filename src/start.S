.section .text
.global _start
.global _dtb_start

_start:
    // Save DTB pointer (from QEMU in x0) into _dtb_start
    adrp x1, _dtb_start        // Get page-aligned address of _dtb_start
    add  x1, x1, :lo12:_dtb_start
    str  x0, [x1]              // Store 64-bit DTB pointer

    // Set up the stack
    ldr x0, =_stack_top
    mov sp, x0

    // Enable floating point/SIMD
    mrs x0, CPACR_EL1
    orr x0, x0, #(3 << 20)     // Set FPEN bits to 0b11
    msr CPACR_EL1, x0
    isb                        // Instruction sync barrier

    // Zero FP status/control
    mov x0, #0
    msr FPCR, x0
    msr FPSR, x0

    // Call kmain()
    bl kmain

.halt:
1:  wfe
    b 1b

// Reserve space for _dtb_start and stack
.section .bss
.align 3
_dtb_start:
    .quad 0

.align 12
.space 4096
_stack_top:
