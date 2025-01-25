#include "common.h"
#include <mutex>
#include "..\platform.h"
#include <stdarg.h>

static void set_console_color(LogSeverity severity)
{
	switch (severity)
	{
	case LogSeverity::Debug:
		platform_set_log_color(PlatformLogColor::GREEN);
		break;
	case LogSeverity::Info:
		platform_set_log_color(PlatformLogColor::BLUE);
		break;
	case LogSeverity::Warning:
		platform_set_log_color(PlatformLogColor::YELLOW);
		break;
	case LogSeverity::Error:
		platform_set_log_color(PlatformLogColor::RED);
		break;
	}
}

static const char* get_str(LogSeverity severity)
{
	switch (severity)
	{
	case LogSeverity::Debug:
		return "DEBUG";
	case LogSeverity::Info:
		return "INFO";
	case LogSeverity::Warning:
		return "WARNING";
	case LogSeverity::Error:
		return "ERROR";
	}
}
#pragma warning(disable:4996)
void _g_logger_log(LogSeverity severity, const char* format, ...)
{
	std::lock_guard<std::mutex> lock(_g_logMutex);
	
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char buf[20] = { 0 };
	std::strftime(buf, sizeof(buf), "%Y-%m-%d %I:%M:%S", std::localtime(&now));
	printf("[%s]", buf);

	printf("[");

	set_console_color(severity);

	printf(get_str(severity));

	platform_reset_log_color();

	printf("]: ");

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\n");
}

int random(int min, int max)
{
	return (rand() % (max - min + 1)) + min;
}

