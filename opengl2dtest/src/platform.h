#pragma once
#include <GLFW/glfw3.h>
#include "util/common.h"

enum PlatformScrollDirection
{
	SCROLL_UP,
	SCROLL_DOWN
};

struct PlatformSoundSettings
{
	float volume;
};

enum PlatformLogColor
{
	GREEN,
	BLUE,
	YELLOW,
	RED
};

typedef void (*scroll_callback)(PlatformScrollDirection direction);
typedef void (*main_loop_callback)();

void platform_post_setup(GLFWwindow* window);
void platform_set_scroll_callback(void* user_data, scroll_callback callback);
void platform_set_main_loop(GLFWwindow* window, main_loop_callback callback);

void platform_sound_init();
void platform_sound_play(const char* path);

void platform_set_log_color(PlatformLogColor color);
void platform_reset_log_color();
