#ifndef __LOG_H__
#define __LOG_H__

/* Simple logger with a couple of logging levels */

#include <stdarg.h>

typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN  = 1,
    LOG_LEVEL_INFO  = 2,
    LOG_LEVEL_DEBUG = 3,
} LogLevel;

void set_log_level(LogLevel level);
void _log(LogLevel level, char* format, ...);

#define LOG_ERROR(str, ...) _log(LOG_LEVEL_ERROR, (str), ##__VA_ARGS__)
#define LOG_WARN(str, ...) _log(LOG_LEVEL_WARN, (str), ##__VA_ARGS__)
#define LOG_INFO(str, ...) _log(LOG_LEVEL_INFO, (str), ##__VA_ARGS__)
#define LOG_DEBUG(str, ...) _log(LOG_LEVEL_DEBUG, (str), ##__VA_ARGS__)


#endif // __LOG_H__
