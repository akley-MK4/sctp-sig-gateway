#include "logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <cstdio>
#include <cstdarg>

static spdlog::level::level_enum parse_level(const char* level) {
    if (!level) return spdlog::level::info;

    if (strcasecmp(level, "trace") == 0)    return spdlog::level::trace;
    if (strcasecmp(level, "debug") == 0)    return spdlog::level::debug;
    if (strcasecmp(level, "info") == 0)     return spdlog::level::info;
    if (strcasecmp(level, "warn") == 0)     return spdlog::level::warn;
    if (strcasecmp(level, "warning") == 0)  return spdlog::level::warn;
    if (strcasecmp(level, "error") == 0)    return spdlog::level::err;
    if (strcasecmp(level, "critical") == 0) return spdlog::level::critical;
    if (strcasecmp(level, "off") == 0)      return spdlog::level::off;

    fprintf(stderr, "[WARN] Unknown log level '%s', fallback to 'info'\n", level);
    return spdlog::level::info;
}

static void log_impl(spdlog::level::level_enum level,
                     const char* file, int line, const char* func,
                     const char* fmt, va_list ap) {
    char full_msg[2304];
    int off = snprintf(full_msg, sizeof(full_msg), "[%s:%d %s] ", file, line, func);
    if (off < 0) off = 0;
    vsnprintf(full_msg + off, sizeof(full_msg) - off, fmt, ap);
    spdlog::log(level, full_msg);
}

void log_init(const char* level) {
    auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("gw", sink);
    logger->set_pattern("[%^%l%$] [tid:%t] %v");
    logger->set_level(parse_level(level));
    spdlog::set_default_logger(logger);
}

void log_shutdown(void) {
    spdlog::shutdown();
}

void log_trace_impl   (const char* f, int l, const char* fn, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); log_impl(spdlog::level::trace, f, l, fn, fmt, ap); va_end(ap); }
void log_debug_impl   (const char* f, int l, const char* fn, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); log_impl(spdlog::level::debug, f, l, fn, fmt, ap); va_end(ap); }
void log_info_impl    (const char* f, int l, const char* fn, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); log_impl(spdlog::level::info,  f, l, fn, fmt, ap); va_end(ap); }
void log_warn_impl    (const char* f, int l, const char* fn, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); log_impl(spdlog::level::warn,  f, l, fn, fmt, ap); va_end(ap); }
void log_error_impl   (const char* f, int l, const char* fn, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); log_impl(spdlog::level::err,   f, l, fn, fmt, ap); va_end(ap); }
void log_critical_impl(const char* f, int l, const char* fn, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); log_impl(spdlog::level::critical, f, l, fn, fmt, ap); va_end(ap); }
