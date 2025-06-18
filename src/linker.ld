ENTRY(_start)

SECTIONS {
    . = 0x40100000; /* Must avoid overlapping with DTB at 0x40000000 */
    _dtb_end_ = .;
    _kernel_start = .;    

    .startup . : {
        start.o(.text)
    }
    /* Code section */
    .text : ALIGN(4K) {
        KEEP(*(.text._kernel_start)) /* needed for heap */
        KEEP(*(.text._start)) /* Ensure _start is first */
        *(.text*)
    }

    /* Read-only data */
    .rodata : ALIGN(4K) {
        *(.rodata*)
    }

    /* Initialized data */
    .data : ALIGN(4K) {
        *(.data*)
    }

    /* Uninitialized data (BSS) */
    .bss : ALIGN(4K) {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    /* Kernel stack (16 KB) */
    . = ALIGN(16);
    _stack_top = .;
    .stack : {
        . = . + 0x4000; /* 16 KB */
    }
    _stack_bottom = .;

    /* End of kernel image */
    _end = .;
}
