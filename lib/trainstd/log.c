#include "log.h"
#include "kern/dev/uart.h"

LogLevel log_level;
LogMask log_mask;

void
log_init(void)
{
    log_level = LOG_LEVEL_WARN;
    log_mask = LOG_MASK_KERN;
}

void
set_log_level(LogLevel level)
{
    log_level = level;
}

LogLevel
get_log_level()
{
    return log_level;
}

void
set_log_mask(LogMask mask)
{
    log_mask = mask;
}

LogMask
get_log_mask(void)
{
    return log_mask;
}

void
_log(LogLevel level, LogMask mask, char* prefix, char* format, ...)
{
    if (level <= log_level && (log_mask & mask) == mask) {
        va_list args;
        va_start(args, format);

        uart_printf(CONSOLE, prefix);
        uart_format_print(CONSOLE, format, args);
        uart_printf(CONSOLE, "\033[0m\r\n");

        va_end(args);
    }
}

void
_panic(void)
{
    // make kernel block forever
    for (;;) {}
}
