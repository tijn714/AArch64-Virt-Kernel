#ifndef UART_H
#define UART_H

#include "common/common.h"

void uart_putc(const char c);
int uart_printk(const char *fmt, ...);

#endif // UART_H