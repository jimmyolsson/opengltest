#include "win32_platform.h"
#if defined(_MSC_VER)
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <Windows.h>

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
#endif
