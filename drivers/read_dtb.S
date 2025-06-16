.section .text
.global read_dtb_uint32
.align 4

// Function: read_dtb_uint32
// Input: x0 - 64-bit address to read from
// Output: w0 - 32-bit value read (converted from big-endian to little-endian)
// Clobbers: x1
read_dtb_uint32:
    // Ensure address is 4-byte aligned
    and x1, x0, #3
    cbz x1, aligned
    mov w0, #0          // Return 0 on misalignment
    ret

aligned:
    ldr w0, [x0]        // Load raw big-endian 32-bit value
    rev w0, w0          // Convert to little-endian
    ret
