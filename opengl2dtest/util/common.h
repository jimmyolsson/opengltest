#pragma once
//#define NOLOG
#ifdef NOLOG
#define TIMER_START(timer_name)
#define TIMER_END(timer_name)
#define g_logger_debug(format, ...)
#define g_logger_info(format, ...)
#define g_logger_warning(format, ...)
#define g_logger_error(format, ...)
#define g_log_bulk(severity, format, ...)
#define g_log_bulk_flush()
#else
#include <chrono>
#define TIMER_START(timer_name) auto timer_name = std::chrono::high_resolution_clock::now();
#define TIMER_END(timer_name) auto timer_name##_end = std::chrono::high_resolution_clock::now(); \
	std::chrono::duration<double, std::milli> timer_name##_dur = timer_name##_end - timer_name; \
	g_logger_debug("TIMER::%s: %fms", #timer_name, timer_name##_dur.count());

#define g_logger_debug(format, ...) _g_logger_log(LogSeverity::Debug, format, __VA_ARGS__)
#define g_logger_info(format, ...) _g_logger_log(LogSeverity::Info, format, __VA_ARGS__)
#define g_logger_warning(format, ...) _g_logger_log(LogSeverity::Warning, format, __VA_ARGS__)
#define g_logger_error(format, ...) _g_logger_log(LogSeverity::Error, format, __VA_ARGS__)
#define g_log_bulk(severity, format, ...) _g_logger_log(severity, format, __VA_ARGS__)
#define g_log_bulk_flush()
#endif // NOLOG

#include <mutex>
static std::mutex _g_logMutex;

enum class LogSeverity
{
	Debug = 0,
	Info,
	Warning,
	Error
};

void _g_logger_log(LogSeverity severity, const char* format, ...);

