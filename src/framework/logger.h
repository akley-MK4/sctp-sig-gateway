#ifndef LOGGER_H_
#define LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* level: "trace", "debug", "info", "warn", "error", "critical", "off" */
void log_init(const char* level);
void log_shutdown(void);

void log_trace_impl(const char* file, int line, const char* func, const char* fmt, ...);
void log_debug_impl(const char* file, int line, const char* func, const char* fmt, ...);
void log_info_impl (const char* file, int line, const char* func, const char* fmt, ...);
void log_warn_impl (const char* file, int line, const char* func, const char* fmt, ...);
void log_error_impl(const char* file, int line, const char* func, const char* fmt, ...);
void log_critical_impl(const char* file, int line, const char* func, const char* fmt, ...);

#define log_trace(fmt, ...)   log_trace_impl(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...)   log_debug_impl(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)    log_info_impl (__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)    log_warn_impl (__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...)   log_error_impl(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define log_critical(fmt, ...) log_critical_impl(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif