#ifndef __LOG_H__
#define __LOG_H__

/* Simple logger with a couple of logging levels */

#include <traindef.h>
#include <stdarg.h>

typedef enum {
    LOG_LEVEL_ALWAYS = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN  = 2,
    LOG_LEVEL_INFO  = 3,
    LOG_LEVEL_DEBUG = 4,
} LogLevel;

typedef u8 LogMask;
typedef enum {
    LOG_MASK_ALL  = 0b00000000,
    LOG_MASK_USER = 0b00000001,
    LOG_MASK_KERN = 0b00000010,
} LogMaskBits;

void log_init(void);
void set_log_level(LogLevel level);
LogLevel get_log_level(void);
void set_log_mask(LogMask log_mask);
LogMask get_log_mask(void);
void _log(LogLevel level, LogMask mask, char* prefix, char* format, ...);

#define KLOG_ERROR(str, ...) _log(LOG_LEVEL_ERROR, LOG_MASK_KERN, "\033[31m[ERROR] ", (str), ##__VA_ARGS__)
#define KLOG_WARN(str, ...) _log(LOG_LEVEL_WARN, LOG_MASK_KERN, "\033[33m[WARN] ", (str), ##__VA_ARGS__)
#define KLOG_INFO(str, ...) _log(LOG_LEVEL_INFO, LOG_MASK_KERN, "\033[36m[INFO] ", (str), ##__VA_ARGS__)
#define KLOG_DEBUG(str, ...) _log(LOG_LEVEL_DEBUG, LOG_MASK_KERN, "[DEBUG] ", (str), ##__VA_ARGS__)

#define PRINT(str, ...) _log(LOG_LEVEL_ALWAYS, LOG_MASK_ALL, "", (str), ##__VA_ARGS__)

#define PANIC(str, ...) _log(LOG_LEVEL_ERROR, LOG_MASK_ALL, "\033[30m\033[41m[PANIC] ", (str), ##__VA_ARGS__); _panic();
void _panic(void);

#endif // __LOG_H__
