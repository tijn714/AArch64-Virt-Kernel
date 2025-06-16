#include <common/common.h>
#include <lib/string.h>
#include <kernel/uart.h>

volatile uint8_t *uart = (uint8_t *) 0x09000000; // Base address QEMU Virt UART 

void uart_init(void *base) {
    uart = (volatile uint8_t *)base;
}

void uart_putc(const char c) {
    *uart = c;
}

static void uart_puts(const char* s) {
    while (*s) {
        uart_putc(*s++);
    }
}

static void print_number(uint64_t value, int base, int width, bool zero_pad, bool upper) {
    char buffer[65];
    char* ptr = &buffer[64];
    const char* digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    *ptr = '\0';

    if (value == 0) {
        *(--ptr) = '0';
    } else {
        while (value) {
            *(--ptr) = digits[value % base];
            value /= base;
        }
    }

    int len = &buffer[64] - ptr;
    while (len < width) {
        uart_putc(zero_pad ? '0' : ' ');
        len++;
    }

    uart_puts(ptr);
}

int uart_printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const char* p = fmt;

    while (*p) {
        if (*p == '%') {
            p++;
            bool zero_pad = false;
            int width = 0;
            int length = 0; // 0: default, 1: l, 2: ll

            // Parse flags
            if (*p == '0') {
                zero_pad = true;
                p++;
            }

            // Parse width
            while (*p >= '0' && *p <= '9') {
                width = width * 10 + (*p - '0');
                p++;
            }

            // Parse length modifiers
            if (*p == 'l') {
                length = 1;
                p++;
                if (*p == 'l') {
                    length = 2;
                    p++;
                }
            }

            char spec = *p++;
            switch (spec) {
                case 'd':
                case 'i': {
                    int64_t value;
                    if (length == 2)
                        value = va_arg(args, int64_t);
                    else if (length == 1)
                        value = va_arg(args, long);
                    else
                        value = va_arg(args, int);

                    if (value < 0) {
                        uart_putc('-');
                        value = -value;
                    }
                    print_number((uint64_t)value, 10, width, zero_pad, false);
                    break;
                }
                case 'u': {
                    uint64_t value;
                    if (length == 2)
                        value = va_arg(args, uint64_t);
                    else if (length == 1)
                        value = va_arg(args, unsigned long);
                    else
                        value = va_arg(args, unsigned int);

                    print_number(value, 10, width, zero_pad, false);
                    break;
                }
                case 'x':
                case 'X': {
                    uint64_t value;
                    if (length == 2)
                        value = va_arg(args, uint64_t);
                    else if (length == 1)
                        value = va_arg(args, unsigned long);
                    else
                        value = va_arg(args, unsigned int);

                    print_number(value, 16, width, zero_pad, spec == 'X');
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    uart_putc(c);
                    break;
                }
                case 's': {
                    const char* s = va_arg(args, const char*);
                    uart_puts(s ? s : "(null)");
                    break;
                }
                case '%': {
                    uart_putc('%');
                    break;
                }
                default:
                    uart_putc('%');
                    uart_putc(spec);
                    break;
            }
        } else {
            uart_putc(*p++);
        }
    }

    va_end(args);
    return 0;
}