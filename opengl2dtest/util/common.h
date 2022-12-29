#pragma once
#define NOLOG

#ifdef NOLOG
#define LOG_DEBUG(msg)
#define _WRITELINE(msg)
#define TIMER_START(timer_name)
#define TIMER_END(timer_name)
#else
#include <chrono>
#define TIMER_START(timer_name) auto timer_name = std::chrono::high_resolution_clock::now();
#define TIMER_END(timer_name) auto timer_name##_end = std::chrono::high_resolution_clock::now(); \
	std::chrono::duration<double, std::milli> timer_name##_dur = timer_name##_end - timer_name; \
	std::cout << "TIMER::" << #timer_name << ": " << timer_name##_dur.count() << "ms\n";
#endif

