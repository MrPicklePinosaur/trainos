#include "./trainstd.h"

// TODO not a big fan of reaching out of lib directory into kernel
#include "kern/uart.h"
#include "kern/alloc.h"

// this is kinda dumb repeated code
void
println(char* format, ...)
{
    va_list args;
    va_start(args, format);

    uart_format_print(CONSOLE, format, args);
    uart_printf(CONSOLE, "\r\n");

    va_end(args);
}

void
print(char* format, ...)
{
    va_list args;
    va_start(args, format);

    uart_format_print(CONSOLE, format, args);

    va_end(args);
}

char
getc(void)
{
    return uart_getc(CONSOLE);
}
