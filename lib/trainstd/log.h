#ifndef __TRAINSTD_LOG_H__
#define __TRAINSTD_LOG_H__

/* Simple logger with a couple of logging levels */

#include <traindef.h>
#include <trainsys.h>
#include <stdarg.h>

typedef enum {
    LOG_LEVEL_ALWAYS = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN  = 2,
    LOG_LEVEL_INFO  = 3,
    LOG_LEVEL_DEBUG = 4,
} LogLevel;

typedef u16 LogMask;
typedef enum {
    LOG_MASK_ALL     = 0,
    LOG_MASK_USER    = 1,
    LOG_MASK_KERN    = 2,

    LOG_MASK_SYSCALL = 4,
    LOG_MASK_ISR     = 8,
    LOG_MASK_MSG     = 16,
    LOG_MASK_SCHED   = 32,

    LOG_MASK_IO      = 64,
    LOG_MASK_NS      = 128,
    LOG_MASK_CLOCK   = 256,
    LOG_MASK_PARSER  = 512,
    LOG_MASK_TRAINSTATE = 1024,
    LOG_MASK_PATH    = 2048,
    LOG_MASK_SENSOR  = 4096,
    LOG_MASK_SWITCH  = 8192
} LogMaskBits;

typedef enum {
    LOG_MODE_STANDARD   = 0,
    LOG_MODE_TRAIN_TERM = 1,
} LogMode;

void log_init(void);
void set_log_level(LogLevel level);
LogLevel get_log_level(void);
void set_log_mask(LogMask mask);
LogMask get_log_mask(void);
void set_log_mode(LogMode mode);
void _log(LogLevel level, LogMask mask, char* prefix, char* format, ...);

#define ULOG_ERROR(str, ...) _log(LOG_LEVEL_ERROR, LOG_MASK_USER, "\033[31m[ERROR] ", (str), ##__VA_ARGS__)
#define ULOG_WARN(str, ...) _log(LOG_LEVEL_WARN, LOG_MASK_USER, "\033[33m[WARN] ", (str), ##__VA_ARGS__)
#define ULOG_INFO(str, ...) _log(LOG_LEVEL_INFO, LOG_MASK_USER, "\033[36m[INFO] ", (str), ##__VA_ARGS__)
#define ULOG_DEBUG(str, ...) _log(LOG_LEVEL_DEBUG, LOG_MASK_USER, "[DEBUG] ", (str), ##__VA_ARGS__)

#define ULOG_INFO_M(mask, str, ...) _log(LOG_LEVEL_INFO, mask|LOG_MASK_USER, "\033[36m[INFO] ", (str), ##__VA_ARGS__)
#define ULOG_DEBUG_M(mask, str, ...) _log(LOG_LEVEL_DEBUG, mask|LOG_MASK_USER, "[DEBUG] ", (str), ##__VA_ARGS__)

#define KLOG_ERROR(str, ...) _log(LOG_LEVEL_ERROR, LOG_MASK_KERN, "\033[31m[ERROR] ", (str), ##__VA_ARGS__)
#define KLOG_WARN(str, ...) _log(LOG_LEVEL_WARN, LOG_MASK_KERN, "\033[33m[WARN] ", (str), ##__VA_ARGS__)
#define KLOG_INFO(str, ...) _log(LOG_LEVEL_INFO, LOG_MASK_KERN, "\033[36m[INFO] ", (str), ##__VA_ARGS__)
#define KLOG_DEBUG(str, ...) _log(LOG_LEVEL_DEBUG, LOG_MASK_KERN, "[DEBUG] ", (str), ##__VA_ARGS__)

#define KLOG_INFO_M(mask, str, ...) _log(LOG_LEVEL_INFO, mask|LOG_MASK_KERN, "\033[36m[INFO] ", (str), ##__VA_ARGS__)
#define KLOG_DEBUG_M(mask, str, ...) _log(LOG_LEVEL_DEBUG, mask|LOG_MASK_KERN, "[DEBUG] ", (str), ##__VA_ARGS__)

#define PRINT(str, ...) _log(LOG_LEVEL_ALWAYS, LOG_MASK_ALL, "", (str), ##__VA_ARGS__)

#define PANIC(str, ...) _log(LOG_LEVEL_ERROR, LOG_MASK_ALL, "\033[30m\033[41m[PANIC] ", (str), ##__VA_ARGS__); _panic();
// Used to mark places where code is unimplemented and is a work in progress
#define UNIMPLEMENTED(str, ...) _log(LOG_LEVEL_ERROR, LOG_MASK_ALL, "\033[30m\033[41m[UNIMPLEMENTED] ", (str), ##__VA_ARGS__); _panic();
// Assert that the code indicated should not be reachable via standard execution
#define UNREACHABLE(str, ...) _log(LOG_LEVEL_ERROR, LOG_MASK_ALL, "\033[30m\033[41m[UNREACHABLE] ", (str), ##__VA_ARGS__); _panic();

void _panic(void);

#endif // __TRAINSTD_LOG_H__
