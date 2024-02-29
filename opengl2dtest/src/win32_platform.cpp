#include "win32_platform.h"
#if defined(_MSC_VER)
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <Windows.h>
#include "irrKlang.h"

static const char* SOUND_PATH = "";
static irrklang::ISoundEngine* sound_engine;

static scroll_callback scroll_cb;
static main_loop_callback main_loop_cb;

void platform_post_setup(GLFWwindow* window)
{
	const int max_width = GetSystemMetrics(SM_CXSCREEN);
	const int max_height = GetSystemMetrics(SM_CYSCREEN);

	int window_width = 0;
	int window_height = 0;
	glfwGetWindowSize(window, &window_width, &window_height);

	// Create the window in the middle
	glfwSetWindowMonitor(window, NULL, (max_width / 2) - (window_width / 2), (max_height / 2) - (window_height / 2), window_width, window_height, GLFW_DONT_CARE);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

void _glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset == -1)
	{
		scroll_cb(PlatformScrollDirection::SCROLL_UP);
	}
	else if (yoffset == 1)
	{
		scroll_cb(PlatformScrollDirection::SCROLL_DOWN);
	}
}

void platform_set_scroll_callback(void* data, scroll_callback callback)
{
	scroll_cb = callback;
	glfwSetScrollCallback((GLFWwindow*)data, _glfw_scroll_callback);
}

void platform_set_main_loop(GLFWwindow* window, main_loop_callback callback)
{
	while (!glfwWindowShouldClose(window))
	{
		callback();
	}
}

void platform_sound_init()
{
	sound_engine = irrklang::createIrrKlangDevice(irrklang::ESOD_AUTO_DETECT, irrklang::ESEO_LOAD_PLUGINS);
	// TODO: Properly log this
	if (!sound_engine)
	{
		return;
	}
	sound_engine->setSoundVolume(0.1);
}

void platform_sound_play(const char* sound_name)
{
	char file_name[100];
	snprintf(file_name, sizeof(file_name), "..\\.\\opengl2dtest\\resources\\sounds\\block_dig\\%s", sound_name);

	// TODO: Do not use hardcoded path
	// This will silently(lol) fail if it dosent find the file 
	sound_engine->play2D(file_name, false);
}

#endif
