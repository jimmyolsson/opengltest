#ifndef __EMSCRIPTEN__
#include "irrKlang.h"
#include <Windows.h>
#include "sound_manager.h"
#endif

#include "util/common_graphics.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/norm.hpp>

#include <iostream>
#include <tuple>
#include <execution>
#include <cmath>
#include <array>
#include <chrono>

#include "common.h"

#include "util/common.h"
#include "graphics/renderer.h"
#include "robin_hood.h"
#include "graphics/camera.h"
#include "chunk.h"
#include "player.h"
#include "blocks/block.h"
#include "util/memory_arena.h"
#include "ui/crosshair.h"
#include "outline.h"
#include "ray.h"
#include "ui/item_toolbar.h"
#include "world_gen.h"
#include "environment/skybox.h"

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
	Skybox skybox;

	player_s player;
	chunk_map_t chunks;
	//sound_manager_s sound_manager;

	memory_arena block_arena;
	memory_arena noise_arena;

	float delta_time = 0.0f;	// time between current frame and last frame
	float last_frame = 0.0f;

	int texture_slot_counter = 0;
} GameState;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLFWwindow* init_and_create_window();
void set_opengl_constants();

void state_allocate_memory()
{
	const auto block_arena_size = (sizeof(block) * BLOCKS_IN_CHUNK) * CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE;
	const auto noise_arena_size = sizeof(float) * BLOCKS_IN_CHUNK * CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE;

	const int mb = 1024 * 1024;
	g_logger_info("Allocating: %dMB", block_arena_size / mb);
	g_logger_info("Allocating: %dMB", noise_arena_size / mb);

	memory_arena_init(&GameState.block_arena, block_arena_size, sizeof(block) * BLOCKS_IN_CHUNK);
	memory_arena_init(&GameState.noise_arena, noise_arena_size, sizeof(float) * (CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT * CHUNK_SIZE_WIDTH));
}

void state_global_init()
{
	// Start 5 blocks over the ground
	glm::ivec3 camera_pos = glm::vec3(0.0f, CHUNK_SIZE_HEIGHT / 6 + 5, 0.0f);
	GameState.player.camera = Camera(camera_pos);
	g_logger_debug("Camera created at (%d, %d, %d)", camera_pos.x, camera_pos.y, camera_pos.z);

	GameState.window = init_and_create_window();
	g_logger_debug("Window created");

	GameState.renderer = renderer_create();
	g_logger_debug("Renderer created");

	GameState.chunks = {};

	GameState.outline = outline_create();
	g_logger_debug("Outline created");

	GameState.skybox = skybox_create();
	g_logger_debug("Skybox created");

	GameState.delta_time = 0;
	GameState.last_frame = 0;

#ifndef __EMSCRIPTEN__
	//sound_init(&GameState.sound_manager);
#endif

	GameState.menu = menu_create(GameState.SCR_WIDTH, GameState.SCR_HEIGHT);
	g_logger_debug("Menu created");
	GameState.crosshair = crosshair_create(GameState.SCR_WIDTH, GameState.SCR_HEIGHT);
	g_logger_debug("Crosshair created");

	state_allocate_memory();
}

void create_and_init_chunk(const int x, const int z)
{
	Chunk chunk;

	chunk.blocks = (block*)memory_arena_get(&GameState.block_arena);
	chunk.chunks = &GameState.chunks;
	chunk.initialized = false;
	glm::ivec2 pos = glm::vec2(x * CHUNK_SIZE_WIDTH, z * CHUNK_SIZE_WIDTH);
	chunk.world_pos = pos;
	GameState.chunks[pos] = chunk;
}

void init_game_world()
{
	g_logger_debug("Initializing game world");
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

#ifndef __EMSCRIPTEN__
	std::for_each(std::execution::par_unseq, std::begin(GameState.chunks), std::end(GameState.chunks),
		[&](auto& iter)
		{
			world_generate(iter.second.blocks, nullptr, iter.first.x, iter.first.y, CHUNK_SIZE_WIDTH, CHUNK_SIZE_HEIGHT);
		});

	std::for_each(std::execution::par_unseq, std::begin(GameState.chunks), std::end(GameState.chunks),
		[&](auto& iter)
		{
			chunk_generate_mesh(&iter.second);
			iter.second.initialized = true;
		});
	for (auto& iter : GameState.chunks)
	{
		chunk_generate_buffers(&iter.second);
	}
#else
	g_logger_debug("Chunk size width: %d - height: %d", CHUNK_SIZE_WIDTH, CHUNK_SIZE_HEIGHT);
	for (auto& iter : GameState.chunks)
	{
		world_generate(iter.second.blocks, nullptr, iter.first.x, iter.first.y, CHUNK_SIZE_WIDTH, CHUNK_SIZE_HEIGHT);
	}

	for (auto& iter : GameState.chunks)
	{
		chunk_generate_mesh(&iter.second);
		chunk_generate_buffers(&iter.second);

		iter.second.initialized = true;
	}

#endif // !__EMSCRIPTEN__
	g_logger_debug("Chunks initialized!");
	int total_verts = 0;
	int total_blocks = 0;
	for (auto& iter : GameState.chunks)
	{
		total_verts += iter.second.verts_in_use + iter.second.verts_in_use_transparent;
		for (int i = 0; i < BLOCKS_IN_CHUNK; i++)
		{
			if (iter.second.blocks[i].type != BlockType::AIR)
				total_blocks++;
		}
	}

	g_logger_info("Done initializing game world!");
	g_logger_info("Total blocks: %d", total_blocks);
	g_logger_info("Total triangles: %d", total_verts / 3);
}

// TODO: Cleanup this disgusting method
void handle_block_hit(ray_hit_result ray_hit, bool remove)
{
	if (ray_hit.chunk_hit == nullptr)
		return;

	BlockType type = BlockType::GLASS;
	if (GameState.menu.item_selected == 0)
	{
		type = BlockType::STONE;
	}
	else if (GameState.menu.item_selected == 1)
	{
		type = BlockType::WATER;
	}
	else if (GameState.menu.item_selected == 2)
	{
		type = BlockType::GLASS;
	}

	if (ray_hit.top_half)
	{
		g_logger_debug("TOP HALF");
	}

	// what chunk and block to process
	Chunk* hit_chunk = nullptr;
	glm::ivec3 b_pos = ray_hit.block_pos;

	g_logger_debug("BLOCK HIT: x:%d y:%d z:%d", b_pos.x, b_pos.y, b_pos.z);

	bool another_chunk = false;
	if (!remove)
	{
		b_pos += ray_hit.direction;
		glm::ivec3 new_pos = ray_hit.block_pos + ray_hit.direction;
		if (new_pos.x >= CHUNK_SIZE_WIDTH)
		{
			hit_chunk = ray_hit.chunk_hit->right_neighbor;
			b_pos.x -= CHUNK_SIZE_WIDTH;
			another_chunk = true;
		}
		else if (new_pos.x < 0)
		{
			hit_chunk = ray_hit.chunk_hit->left_neighbor;
			b_pos.x += CHUNK_SIZE_WIDTH;
			another_chunk = true;
		}
		else if (new_pos.z < 0)
		{
			hit_chunk = ray_hit.chunk_hit->back_neighbor;
			b_pos.z += CHUNK_SIZE_WIDTH;
			another_chunk = true;
		}
		else if (new_pos.z >= CHUNK_SIZE_WIDTH)
		{
			hit_chunk = ray_hit.chunk_hit->front_neighbor;
			b_pos.z -= CHUNK_SIZE_WIDTH;
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

	if (another_chunk)
	{
		//sound_play_block_sound(&GameState.sound_manager, type, remove);
		chunk_set_block(hit_chunk, b_pos, type);
	}
	else
	{
		if (remove)
		{
			//sound_play_block_sound(&GameState.sound_manager, chunk_get_block(hit_chunk, b_pos)->type, remove);
			chunk_set_block(hit_chunk, b_pos, BlockType::AIR);
		}
		else
		{
			chunk_set_block(hit_chunk, b_pos, type);
			//if (type != BlockType::AIR)
				//sound_play_block_sound(&GameState.sound_manager, type, remove);
		}

		hit_chunk->verts_in_use = 0;
		hit_chunk->verts_in_use_transparent = 0;
		hit_chunk->dirty = true;
	}
}

void opengl_clear_screen()
{
	GL_CALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
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


void render_3d()
{
	glm::mat4 view = GameState.player.camera.GetViewMatrix();
	// Render the opaque objects first
	for (auto& iter : GameState.chunks)
	{
		glm::vec3 position = glm::vec3(iter.first.x, 0, iter.first.y);
		chunk_render_opaque(&iter.second, &GameState.renderer, view, position);
	}

	outline_render(&GameState.outline, &GameState.renderer, view);

	// Render translucent objects
	// TODO: Sort
	for (auto& iter : GameState.chunks)
	{
		glm::vec3 position = glm::vec3(iter.first.x, 0, iter.first.y);
		chunk_render_transparent(&iter.second, &GameState.renderer, view, position);
	}
}

void render_2d()
{
	const glm::mat4 view = glm::mat4(1.0f);
	menu_render(&GameState.menu, &GameState.renderer, view, GameState.SCR_WIDTH, GameState.SCR_HEIGHT);
	crosshair_render(&GameState.crosshair, &GameState.renderer, view, GameState.SCR_WIDTH, GameState.SCR_HEIGHT);
}

void game_render()
{
	// Skybox render
	{
		GL_CALL(glEnable(GL_DEPTH_TEST));
		GL_CALL(glDepthFunc(GL_LEQUAL));
		skybox_render(&GameState.skybox, &GameState.renderer, GameState.player.camera.GetViewMatrix());
	}

	// 3D pass
	{
		GL_CALL(glEnable(GL_BLEND));
		GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		GL_CALL(glEnable(GL_CULL_FACE));
		GL_CALL(glCullFace(GL_BACK));

		GL_CALL(glEnable(GL_DEPTH_TEST));
		GL_CALL(glDepthFunc(GL_LESS));

		render_3d();
	}

	// 2D pass
	{
		GL_CALL(glDisable(GL_CULL_FACE));
		GL_CALL(glDisable(GL_DEPTH_TEST));
		render_2d();
	}
}

void game_update()
{
	const float projection_fov = glm::radians(GameState.player.camera.Zoom);// * (GameState.SCR_WIDTH / GameState.SCR_HEIGHT);
	const float projection_aspect = GameState.SCR_WIDTH / GameState.SCR_HEIGHT;

	const glm::mat4 perspective = glm::perspective(projection_fov, projection_aspect, 0.1f, 2000.0f);
	const glm::mat4 orthographic = glm::ortho(0.0f, GameState.SCR_WIDTH, 0.0f, GameState.SCR_HEIGHT, -100.0f, 100.0f);

	renderer_update(&GameState.renderer, perspective, orthographic);
	outline_update(&GameState.outline, GameState.player.camera.Position, GameState.player.camera.Front, &GameState.chunks);
	chunk_update(&GameState.chunks);
}

void em_loop()
{
#ifdef __EMSCRIPTEN__
	static float last_frame = 0;

	// calculate delta time
	float current_frame = emscripten_get_now() / 1000.0;
	float delta_time = current_frame - last_frame;
	last_frame = current_frame;

	glfwPollEvents();

	processInput(GameState.window, delta_time);

	game_update();

	opengl_clear_screen();
	game_render();

	glfwSwapBuffers(GameState.window);
#endif
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
	// Global configurations
	srand(time(NULL));
	std::ios_base::sync_with_stdio(false);

#ifndef __EMSCRIPTEN__
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0F);
#endif

	state_global_init();

	init_game_world();
	memory_arena_dealloc(&GameState.noise_arena);

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(em_loop, 0, 1);
#else
	game_main_loop();
#endif

	return 0;
}

#pragma region INIT_OPENGL
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
		}
	}
	if (yoffset == 1)
	{
		if (GameState.menu.item_selected != 0)
		{
			last_x = GameState.menu.highlight.position;
			GameState.menu.item_selected--;

			GameState.menu.highlight.position = glm::vec2((GameState.menu.toolbar.position.x - 5) + step * GameState.menu.item_selected, 0);
		}
	}
}

GLFWwindow* init_and_create_window()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(GameState.SCR_WIDTH, GameState.SCR_HEIGHT, "opengltest", NULL, NULL);
	if (window == NULL)
	{
		g_logger_error("Failed to create GLFW window");
		glfwTerminate();
		return nullptr;
	}
#ifndef __EMSCRIPTEN__
	int max_width = GetSystemMetrics(SM_CXSCREEN);
	int max_height = GetSystemMetrics(SM_CYSCREEN);
	glfwSetWindowMonitor(window, NULL, (max_width / 2) - (GameState.SCR_WIDTH / 2), (max_height / 2) - (GameState.SCR_HEIGHT / 2), GameState.SCR_WIDTH, GameState.SCR_HEIGHT, GLFW_DONT_CARE);
#endif

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

#ifndef __EMSCRIPTEN__
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		g_logger_error("Failed to initialize GLAD");
		return nullptr;
	}
#endif

	g_logger_info("OpenGL version: %s", glGetString(GL_VERSION));
	return window;
}
#pragma endregion

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	GameState.SCR_WIDTH = width;
	GameState.SCR_HEIGHT = height;
	GL_CALL(glViewport(0, 0, width, height));
}