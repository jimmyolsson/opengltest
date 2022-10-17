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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

#include "robin_hood.h"
#include "camera.h"
#include "chunk.h"
#include "player.h"
#include "shader_m.h"
#include "blocks/block.h"
#include "memory_arena.h"
#include "crosshair.h"
#include "outline.h"
#include "sound_manager.h"
#include "ray.h"
#include "item_toolbar.h"
#include "world_gen.h"

const int SHADER_COUNT = 3;
struct State
{
	GLFWwindow* window;

	const float SCR_WIDTH = 1360;
	const float SCR_HEIGHT = 960;

	ItemToolbar menu;
	crosshair_t crosshair;
	outline_block outline;

	player_s player;
	chunk_map_t chunks;
	sound_manager_s sound_manager;

	//shader_program shaders[SHADER_COUNT];

	memory_arena block_arena;
	memory_arena chunk_arena;
	memory_arena noise_arena;

	float delta_time = 0.0f;	// time between current frame and last frame
	float last_frame = 0.0f;
} GameState;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
GLFWwindow* init_and_create_window();
GLuint load_textures();
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
	GameState.player.camera = Camera(glm::vec3(0.0f, 150.0f, 0.0f));
	GameState.window = init_and_create_window();
	GameState.chunks = {};

	GameState.crosshair = crosshair_create();
	GameState.outline = outline_create();

	GameState.delta_time = 0;
	GameState.last_frame = 0;

	sound_init(&GameState.sound_manager);

	GameState.menu = menu_create();
	// TODO: This loads from GL_TEXTURE1
	//menu_loadtexture(&GameState.menu);

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


void game_render(Shader* lightingShader, unsigned int atlas)
{
	{
		const float projection_fov = glm::radians(GameState.player.camera.Zoom) * (GameState.SCR_WIDTH / GameState.SCR_HEIGHT);
		const float projection_aspect = GameState.SCR_WIDTH / GameState.SCR_HEIGHT;
		const glm::mat4 projection = glm::perspective(projection_fov, projection_aspect, 0.1f, 1000.0f);
		const glm::mat4 view = GameState.player.camera.GetViewMatrix();
		const glm::mat4 model = glm::mat4(1.0f);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);

		lightingShader->use();

		lightingShader->setMat4("projection", projection);
		lightingShader->setMat4("view", view);
		lightingShader->setMat4("model", model);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, atlas);

		for (auto& iter : GameState.chunks)
		{
			lightingShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(iter.first.x, 0, iter.first.y)));
			chunk_render(iter.second);
		}

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		outline_render(&GameState.outline, projection, view);
	}

	// Render 2D

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	//const glm::mat4 projection_ortho = glm::ortho()
	menu_render(&GameState.menu);
	crosshair_render(&GameState.crosshair, GameState.SCR_WIDTH, GameState.SCR_HEIGHT);
}

void game_update()
{
	outline_update(&GameState.outline, GameState.player.camera.Position, GameState.player.camera.Front, &GameState.chunks);
	chunk_update(&GameState.chunks);
}

void game_main_loop(unsigned int atlas)
{
	Shader lightingShader("..\\resources\\shaders\\opaque_vert.glsl", "..\\resources\\shaders\\opaque_frag.glsl");

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
		game_render(&lightingShader, atlas);

		glfwSwapBuffers(GameState.window);
	}
}

int main()
{
	srand(time(NULL));
	state_global_init();

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

	const auto atlas = load_textures();

	init_chunks();
	memory_arena_dealloc(&GameState.noise_arena);

	game_main_loop(atlas);

	return 0;
}

#pragma region INIT_OPENGL
void set_opengl_constants()
{

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset == -1)
	{
		if(GameState.menu.item_selected != 8)
			GameState.menu.item_selected++;
	}
	if (yoffset == 1)
	{
		if(GameState.menu.item_selected != 0)
			GameState.menu.item_selected--;
	}
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

#pragma region LOAD_TEXTURES
unsigned char* load_png(const char* path)
{
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	//std::cout << nrChannels << "\n";
	return data;
}

GLuint load_textures()
{
	unsigned char* dirt = load_png("..\\resources\\textures\\dirt.png");
	unsigned char* dirt_grass_side = load_png("..\\resources\\textures\\dirt_grass_side.png");
	unsigned char* dirt_grass_top = load_png("..\\resources\\textures\\dirt_grass_top.png");
	unsigned char* stone = load_png("..\\resources\\textures\\stone.png");
	unsigned char* sand = load_png("..\\resources\\textures\\sand.png");
	unsigned char* leaves = load_png("..\\resources\\textures\\leaves.png");
	unsigned char* oak_log = load_png("..\\resources\\textures\\oak_log.png");
	unsigned char* oak_log_top = load_png("..\\resources\\textures\\oak_log_top.png");

	unsigned int atlas;
	GLsizei width = 16;
	GLsizei height = 16;

	// CURRENT NUMBER OF TEXTURES
	GLsizei layerCount = BLOCK_TYPE_LAST + 1;
	GLsizei mipLevelCount = 4;

	glGenTextures(1, &atlas);
	glBindTexture(GL_TEXTURE_2D_ARRAY, atlas);

	// Allocate the storage.
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGBA8, width, height, layerCount);

	// Upload pixel data.
	// The first 0 refers to the mipmap level (level 0, since there's only 1)
	// The following 2 zeroes refers to the x and y offsets in case you only want to specify a subrectangle.
	// The final 0 refers to the layer index offset (we start from index 0 and have 2 levels).
	// Altogether you can specify a 3D box subset of the overall texture, but only one mip level at a time.

	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, stone);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, dirt);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 2, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, dirt_grass_side);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 3, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, dirt_grass_top);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 4, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, sand);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 5, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, leaves);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 6, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, oak_log);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 7, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, oak_log_top);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//// set texture filtering parameters
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//GLfloat value, max_anisotropy = 8.0f;
	//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &value);

	//value = (value > max_anisotropy) ? max_anisotropy : value;
	//glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY, value);

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	return atlas;
}
#pragma endregion

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}