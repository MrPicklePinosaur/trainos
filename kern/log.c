#include "log.h"
#include "rpi.h"

static LogLevel log_level = LOG_LEVEL_WARN;

void
set_log_level(LogLevel level)
{
    log_level = level;
}

void
_log(LogLevel level, char* prefix, char* format, ...)
{
    if (level <= log_level) {
        va_list args;
        va_start(args, format);

        uart_printf(CONSOLE, prefix);
        uart_format_print(CONSOLE, format, args);
        uart_printf(CONSOLE, "\033[0m\r\n");

        va_end(args);
    }
}
