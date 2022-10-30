#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>
#else
#include "glad/glad.h"
#endif

#ifdef __EMSCRIPTEN__
//#include <GL/glfw.h>
#include <GLFW/glfw3.h>
#else
#include "irrKlang.h"
#include "GLFW/glfw3.h"
#endif

#ifdef __EMSCRIPTEN__
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#else
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/norm.hpp>
#endif

#include <iostream>
#include <tuple>
#include <algorithm>
#include <execution>
#include <cmath>
#include <array>
#include <chrono>

#include "common.h"

#include "graphics/renderer.h"
#include "robin_hood.h"
#include "graphics/camera.h"
#include "chunk.h"
#include "player.h"
#include "blocks/block.h"
#include "util/memory_arena.h"
#include "ui/crosshair.h"
#include "outline.h"
#include "sound_manager.h"
#include "ray.h"
#include "ui/item_toolbar.h"
#include "world_gen.h"

const int SHADER_COUNT = 3;
struct State
{
	GLFWwindow* window;
	Renderer renderer;

	float SCR_WIDTH = 1360;
	float SCR_HEIGHT = 960;

	UIToolbar menu;
	UICrosshair crosshair;
	OutlineBlock outline;

	player_s player;
	chunk_map_t chunks;
	sound_manager_s sound_manager;

	memory_arena block_arena;
	memory_arena chunk_arena;
	memory_arena noise_arena;

	float delta_time = 0.0f;	// time between current frame and last frame
	float last_frame = 0.0f;

	int texture_slot_counter = 0;
} GameState;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLFWwindow* init_and_create_window();
void set_opengl_constants();

void state_allocate_memory_arenas()
{
	auto block_arena_size = (sizeof(block) * BLOCKS_IN_CHUNK) * CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE;
	auto noise_arena_size = sizeof(float) * BLOCKS_IN_CHUNK * CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE;
	auto size_chunk = sizeof(block_size_t) * BLOCKS_IN_CHUNK * CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE;
	int mb = 1024 * 1024;
	std::cout << "block: " << block_arena_size / mb << "MB\n";
	std::cout << "noise: " << noise_arena_size / mb << "MB\n";
	std::cout << "chunk: " << size_chunk / mb << "MB\n";

	memory_arena_init(&GameState.block_arena, block_arena_size);
	memory_arena_init(&GameState.noise_arena, noise_arena_size);
	memory_arena_init(&GameState.chunk_arena, size_chunk);
}

void state_global_init()
{
	GameState.player.camera = Camera(glm::vec3(0.0f, 70.0f, 0.0f));
	GameState.window = init_and_create_window();
	GameState.renderer = renderer_create();
	GameState.chunks = {};

	GameState.outline = outline_create();

	GameState.delta_time = 0;
	GameState.last_frame = 0;

	sound_init(&GameState.sound_manager);

	GameState.menu = menu_create(GameState.SCR_WIDTH, GameState.SCR_HEIGHT);
	GameState.crosshair = crosshair_create(GameState.SCR_WIDTH, GameState.SCR_HEIGHT);

	state_allocate_memory_arenas();
}

void create_and_init_chunk(const int x, const int z)
{
	chunk chunk;

	chunk.blocks = (block*)memory_arena_get(&GameState.block_arena, sizeof(block) * BLOCKS_IN_CHUNK);
	chunk.gpu_data_arr = (block_size_t*)memory_arena_get(&GameState.chunk_arena, (sizeof(block_size_t) * BLOCKS_IN_CHUNK));
	chunk.chunks = &GameState.chunks;
	chunk.initialized = false;
	glm::ivec2 pos = glm::vec2(x * CHUNK_SIZE_WIDTH, z * CHUNK_SIZE_WIDTH);
	chunk.world_pos = pos;

	world_generate(chunk.blocks, &GameState.noise_arena, pos.x, pos.y, CHUNK_SIZE_WIDTH, CHUNK_SIZE_HEIGHT);

	GameState.chunks[pos] = chunk;
}

void init_chunks()
{
	using namespace std::chrono;

	if (CHUNK_DRAW_DISTANCE == 1)
	{
		create_and_init_chunk(0, 0);
	}
	else
	{
		for (int x = (CHUNK_DRAW_DISTANCE / 2) * -1; x < CHUNK_DRAW_DISTANCE / 2; x++)
		{
			for (int z = (CHUNK_DRAW_DISTANCE / 2) * -1; z < CHUNK_DRAW_DISTANCE / 2; z++)
			{
				create_and_init_chunk(x, z);
			}
		}
	}

	// ----------------- MESH GEN -----------------
	std::for_each(std::execution::par_unseq, std::begin(GameState.chunks), std::end(GameState.chunks),
		[&](auto& iter)
		{
			chunk_generate_mesh(&iter.second);
		});

	for (auto& iter : GameState.chunks)
	{
		chunk_generate_buffers(&iter.second);
	}
}

void handle_block_hit(ray_hit_result ray_hit, bool remove)
{
	if (ray_hit.chunk_hit == nullptr)
		return;

	block_type type = block_type::STONE;

	// what chunk and block to process
	chunk* hit_chunk = nullptr;
	glm::ivec3 b_pos = ray_hit.block_pos;

	bool another_chunk = false;
	if (!remove)
	{
		b_pos += ray_hit.direction;
		if ((ray_hit.block_pos + ray_hit.direction).x >= CHUNK_SIZE_WIDTH)
		{
			hit_chunk = ray_hit.chunk_hit->right_neighbor;
			another_chunk = true;
		}
		else if ((ray_hit.block_pos + ray_hit.direction).x <= 0)
		{
			hit_chunk = ray_hit.chunk_hit->left_neighbor;
			another_chunk = true;
		}
		else if ((ray_hit.block_pos + ray_hit.direction).z <= 0)
		{
			hit_chunk = ray_hit.chunk_hit->back_neighbor;
			another_chunk = true;
		}
		else if ((ray_hit.block_pos + ray_hit.direction).z >= CHUNK_SIZE_WIDTH)
		{
			hit_chunk = ray_hit.chunk_hit->front_neighbor;
			another_chunk = true;
		}
		else
		{
			hit_chunk = ray_hit.chunk_hit;
		}
	}
	else
	{
		hit_chunk = ray_hit.chunk_hit;
	}

	using namespace std::chrono;
	if (another_chunk)
	{
		sound_play_block_sound(&GameState.sound_manager, type, remove);
		b_pos = glm::vec3((ray_hit.block_pos.x - (CHUNK_SIZE_WIDTH - 1)), ray_hit.block_pos.y, ray_hit.block_pos.z);
		chunk_set_block(hit_chunk, b_pos, type);
	}
	else
	{
		if (remove)
		{
			sound_play_block_sound(&GameState.sound_manager, chunk_get_block(hit_chunk, b_pos)->type, remove);
			chunk_set_block(hit_chunk, b_pos, block_type::AIR);
		}
		else
		{
			chunk_set_block(hit_chunk, b_pos, type);
			if (type != block_type::AIR)
				sound_play_block_sound(&GameState.sound_manager, type, remove);
		}

		hit_chunk->blocks_in_use = 0;
		hit_chunk->dirty = true;
	}
}

void opengl_clear_screen()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

float lastX = GameState.SCR_WIDTH / 2.0f;
float lastY = GameState.SCR_HEIGHT / 2.0f;

bool lmouse = false;
bool rmouse = false;
void processInput(GLFWwindow* window, double delta_time)
{
	double x, y;

	glfwGetCursorPos(window, &x, &y);
	float xoffset = x - lastX;
	float yoffset = lastY - y;

	lastX = x;
	lastY = y;

	GameState.player.camera.ProcessMouseMovement(xoffset, yoffset);

	glm::vec3 position = GameState.player.camera.Position;
	glm::vec3 rotation = GameState.player.camera.Front;

	bool lcmouse = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	if (!lmouse && lcmouse)
	{
		Ray r(GameState.player.camera.Position, GameState.player.camera.Front);
		ray_hit_result result = r.intersect_block(50, &GameState.chunks);

		handle_block_hit(result, true);
	}
	lmouse = lcmouse;

	bool rcmouse = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	if (!rmouse && rcmouse)
	{
		Ray r(GameState.player.camera.Position, GameState.player.camera.Front);
		ray_hit_result result = r.intersect_block(50, &GameState.chunks);

		handle_block_hit(result, false);
	}
	rmouse = rcmouse;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		GameState.player.camera.ProcessKeyboard(FORWARD, delta_time);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		GameState.player.camera.ProcessKeyboard(BACKWARD, delta_time);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		GameState.player.camera.ProcessKeyboard(LEFT, delta_time);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		GameState.player.camera.ProcessKeyboard(RIGHT, delta_time);
}

void game_render()
{
	// Render 3D 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	glm::mat4 view = GameState.player.camera.GetViewMatrix();
	for (auto& iter : GameState.chunks)
	{
		glm::vec3 position = glm::vec3(iter.first.x, 0, iter.first.y);
		chunk_render(iter.second, &GameState.renderer, view, position);
	}

	outline_render(&GameState.outline, &GameState.renderer, view);

	// Render 2D
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	const glm::mat4 projection_view = glm::mat4(1.0f);
	menu_render(&GameState.menu, &GameState.renderer, projection_view, GameState.SCR_WIDTH, GameState.SCR_HEIGHT);
	crosshair_render(&GameState.crosshair, &GameState.renderer, projection_view, GameState.SCR_WIDTH, GameState.SCR_HEIGHT);
}

void game_update()
{
	const float projection_fov = glm::radians(GameState.player.camera.Zoom);// * (GameState.SCR_WIDTH / GameState.SCR_HEIGHT);
	const float projection_aspect = GameState.SCR_WIDTH / GameState.SCR_HEIGHT;

	const glm::mat4 perspective = glm::perspective(projection_fov, projection_aspect, 0.1f, 1000.0f);
	const glm::mat4 orthographic = glm::ortho(0.0f, GameState.SCR_WIDTH, 0.0f, GameState.SCR_HEIGHT, -100.0f, 100.0f);

	renderer_update(&GameState.renderer, perspective, orthographic);
	outline_update(&GameState.outline, GameState.player.camera.Position, GameState.player.camera.Front, &GameState.chunks);
	chunk_update(&GameState.chunks);
}

void game_main_loop()
{
	float delta_time = 0;
	float last_frame = 0;
	while (!glfwWindowShouldClose(GameState.window))
	{
		// calculate delta time
		float current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;
		glfwPollEvents();

		processInput(GameState.window, delta_time);

		game_update();

		opengl_clear_screen();
		game_render();

		glfwSwapBuffers(GameState.window);
	}
}

int main()
{
	srand(time(NULL));
	state_global_init();

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

	init_chunks();
	memory_arena_dealloc(&GameState.noise_arena);

	game_main_loop();

	return 0;
}

#pragma region INIT_OPENGL
void set_opengl_constants()
{

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

glm::dvec2 last_x;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	double step = 80;
	(GameState.menu.toolbar.position.x - 5) + step * GameState.menu.item_selected;
	if (yoffset == -1)
	{
		if (GameState.menu.item_selected != 8)
		{
			last_x = GameState.menu.highlight.position;
			GameState.menu.item_selected++;

			GameState.menu.highlight.position = glm::vec2((GameState.menu.toolbar.position.x - 5) + step * GameState.menu.item_selected, 0);

			std::cout << last_x.x - GameState.menu.highlight.position.x << "\n";
		}
	}
	if (yoffset == 1)
	{
		if (GameState.menu.item_selected != 0)
		{
			last_x = GameState.menu.highlight.position;
			GameState.menu.item_selected--;

			GameState.menu.highlight.position = glm::vec2((GameState.menu.toolbar.position.x - 5) + step * GameState.menu.item_selected, 0);

			std::cout << (double)((double)last_x.x - (double)GameState.menu.highlight.position.x) << "\n";
		}
	}

	std::cout << GameState.menu.highlight.position.x << "\n";
}

#include <Windows.h>
GLFWwindow* init_and_create_window()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(GameState.SCR_WIDTH, GameState.SCR_HEIGHT, "opengltest", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	int max_width = GetSystemMetrics(SM_CXSCREEN);
	int max_hieght = GetSystemMetrics(SM_CYSCREEN);
	glfwSetWindowMonitor(window, NULL, (max_width / 2) - (GameState.SCR_WIDTH / 2), (max_hieght / 2) - (GameState.SCR_HEIGHT / 2), GameState.SCR_WIDTH, GameState.SCR_HEIGHT, GLFW_DONT_CARE);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

#ifndef __EMSCRIPTEN__
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}
#endif

	return window;
}
#pragma endregion

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	GameState.SCR_WIDTH = width;
	GameState.SCR_HEIGHT = height;
	glViewport(0, 0, width, height);
}