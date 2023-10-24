#ifndef __DEV_UART_H__
#define __DEV_UART_H__

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
int uart_getc_poll(size_t line, unsigned char* data);
void uart_puts(size_t line, const char* buf);

void uart_printf(size_t line, char *fmt, ...);
void uart_format_print (size_t line, char *fmt, va_list va );

bool uart_is_rx_interrupt(size_t line);
bool uart_is_cts_interrupt(size_t line);
bool uart_get_cts(size_t line);
void uart_clear_interrupts(size_t line);

unsigned char uart_getc_buffered(size_t line);

#endif // __DEV_UART_H__
