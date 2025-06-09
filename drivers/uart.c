#include "common/common.h"
#include "lib/string.h"
#include "kernel/uart.h"

volatile uint8_t *uart = (uint8_t *) 0x09000000; // TODO: Parse from DTB

void uart_putc(const char c) {
    *uart = c;
}

static void uart_puts(const char* s) {
    while (*s) {
        uart_putc(*s++);
    }
}

static void print_number(uint64_t value, int base, int width, bool zero_pad, bool upper) {
    char buffer[65]; // max 64 digits + null terminator for base 2
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
            bool long_flag = false;

            if (*p == '0') {
                zero_pad = true;
                p++;
            }
            while (*p >= '0' && *p <= '9') {
                width = width * 10 + (*p - '0');
                p++;
            }
            if (*p == 'l') {
                long_flag = true;
                p++;
                if (*p == 'l') {
                    p++; // support for %ll (optional)
                }
            }

            char spec = *p++;
            switch (spec) {
                case 'd':
                case 'i': {
                    int64_t value = long_flag ? va_arg(args, int64_t) : va_arg(args, int32_t);
                    if (value < 0) {
                        uart_putc('-');
                        value = -value;
                    }
                    print_number((uint64_t)value, 10, width, zero_pad, false);
                    break;
                }
                case 'u': {
                    uint64_t value = long_flag ? va_arg(args, uint64_t) : va_arg(args, uint32_t);
                    print_number(value, 10, width, zero_pad, false);
                    break;
                }
                case 'x':
                case 'X': {
                    uint64_t value = long_flag ? va_arg(args, uint64_t) : va_arg(args, uint32_t);
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
                    uart_puts(s);
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