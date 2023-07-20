#include "common.h"
#include <mutex>
#include <Windows.h>

void set_console_color(LogSeverity severity)
{
	switch (severity)
	{
	case LogSeverity::Debug:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
		break;
	case LogSeverity::Info:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN);
		break;
	case LogSeverity::Warning:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_RED);
		break;
	case LogSeverity::Error:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
		break;
	}
}

const char* get_str(LogSeverity severity)
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
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0F);

	printf("]: ");

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\n");

}
