#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

/* ============================================================
   OmniOS - Kernel Debug / Logging
   ============================================================ */

typedef enum {
    LVL_DEBUG    = 0,
    LVL_INFO     = 1,
    LVL_WARN     = 2,
    LVL_ERROR    = 3,
    LVL_CRITICAL = 4,
} DebugLevel;

#ifndef MIN_LOG_LEVEL
#  define MIN_LOG_LEVEL LVL_DEBUG
#endif

void logf(const char *module, DebugLevel level, const char *fmt, ...);

/* Full name macros */
#define log_debug(mod, fmt, ...)    logf(mod, LVL_DEBUG,    fmt, ##__VA_ARGS__)
#define log_info(mod, fmt, ...)     logf(mod, LVL_INFO,     fmt, ##__VA_ARGS__)
#define log_warn(mod, fmt, ...)     logf(mod, LVL_WARN,     fmt, ##__VA_ARGS__)
#define log_error(mod, fmt, ...)    logf(mod, LVL_ERROR,    fmt, ##__VA_ARGS__)
#define log_critical(mod, fmt, ...) logf(mod, LVL_CRITICAL, fmt, ##__VA_ARGS__)

/* Short aliases used by existing nanobyte_os code */
#define log_err(mod, fmt, ...)      logf(mod, LVL_ERROR,    fmt, ##__VA_ARGS__)
#define log_crit(mod, fmt, ...)     logf(mod, LVL_CRITICAL, fmt, ##__VA_ARGS__)