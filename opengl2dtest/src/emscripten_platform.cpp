#ifdef __EMSCRIPTEN__
#include "emscripten_platform.h"

#include "emscripten/emscripten.h"
#include "emscripten/html5.h"

void platform_post_setup(GLFWwindow* window)
{
}

static scroll_callback scroll_cb;
EM_BOOL em_wheel_callback(int eventType, const EmscriptenWheelEvent* e, void* userData)
{
	if (e->deltaY < 0)
	{
		scroll_cb(PlatformScrollDirection::SCROLL_UP);
	}
	else if (e->deltaY > 0)
	{
		scroll_cb(PlatformScrollDirection::SCROLL_DOWN);
	}

	return EM_TRUE;
}

void platform_set_scroll_callback(void* data, scroll_callback callback)
{
	scroll_cb = callback;
	emscripten_set_wheel_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, em_wheel_callback);
}

void platform_set_main_loop(GLFWwindow* window, main_loop_callback callback)
{
	emscripten_set_main_loop(callback, 0, 1);
}

// Sound currently not supported
void platform_sound_init()
{

}
void platform_sound_play(const char* path)
{

}

// Log color not supported
void platform_set_log_color(PlatformLogColor color)
{

}
void platform_reset_log_color()
{

}
#endif
