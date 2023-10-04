#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

// Serial line 1 on the RPi hat is used for the console
static const size_t CONSOLE = 1;

// Serial line 2 is the train control
static const size_t MARKLIN = 2;

void uart_init();
void uart_putc(size_t line, unsigned char c);
unsigned char uart_getc(size_t line);
void uart_puts(size_t line, const char* buf);

void uart_printf(size_t line, char *fmt, ...);
void uart_format_print (size_t line, char *fmt, va_list va );

#endif // __UART_H__
