#include "trainstd.h"

// TODO not a big fan of reaching out of lib directory into kernel
#include "../../kern/rpi.h"

void
println(char* format, ...)
{
    va_list args;
    va_start(args, format);

    uart_format_print(CONSOLE, format, args);
    uart_printf(CONSOLE, "\r\n");

    va_end(args);
}
