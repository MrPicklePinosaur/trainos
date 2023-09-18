#ifndef _rpi_h_
#define _rpi_h_ 1

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Serial line 1 on the RPi hat is used for the console
static const size_t CONSOLE = 1;

// Serial line 2 is the train control
static const size_t MARKLIN = 2;

void uart_putc(size_t line, unsigned char c);
int uart_try_putc(size_t line, unsigned char c);
unsigned char uart_getc(size_t line);
int uart_getc_poll(size_t line, unsigned char* data);
void uart_putl(size_t line, const char *buf, size_t blen);
void uart_puts(size_t line, const char *buf);
void uart_printf(size_t line, char *fmt, ...);
void uart_config_and_enable(size_t line, uint32_t baudrate, uint32_t control);
void uart_init();
bool uart_busy(size_t line);

uint64_t timer_get(void);

#endif /* rpi.h */
