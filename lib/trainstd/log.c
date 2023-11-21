#include <trainstd.h>
#include <trainsys.h>
#include <traintasks.h>

#include "log.h"
#include "kern/dev/uart.h"

LogLevel log_level;
LogMask log_mask;
LogMode log_mode;

const uint32_t train_term_log_x = 123;
const uint32_t train_term_log_min_y = 2;
const uint32_t train_term_log_max_y = 60;
uint32_t train_term_log_y;

void
log_init(void)
{
    log_level = LOG_LEVEL_WARN;
    log_mask = LOG_MASK_KERN;
    log_mode = LOG_MODE_STANDARD;

    train_term_log_y = train_term_log_min_y;
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
set_log_mode(LogMode mode)
{
    log_mode = mode;
    if (mode != LOG_MODE_TRAIN_TERM) {
        train_term_log_y = train_term_log_min_y;
    }
}

void
_log(LogLevel level, LogMask mask, char* prefix, char* format, ...)
{
    if (level <= log_level && (log_mask & mask) == mask) {
        va_list args;
        va_start(args, format);

        // Special cursor positioning instructions for train term mode
        if (log_mode == LOG_MODE_TRAIN_TERM) {
            // Clear next two lines
            uart_printf(CONSOLE, "\033[%d;%dH\033[0K", train_term_log_y+1, train_term_log_x);
            uart_printf(CONSOLE, "\033[%d;%dH\033[0K", train_term_log_y+2, train_term_log_x);

            // Move cursor to correct line
            uart_printf(CONSOLE, "\033[%d;%dH", train_term_log_y, train_term_log_x);
            train_term_log_y++;
            if (train_term_log_y > train_term_log_max_y) {
                train_term_log_y = train_term_log_min_y;
            }
        }

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
