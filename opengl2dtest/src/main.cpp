#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

#include "util/common_graphics.h"

#include "platform.h"
#ifdef __EMSCRIPTEN__
#include "emscripten_platform.h"
#else
#include "win32_platform.h"
#endif

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

struct State
{
	GLFWwindow* window;
	Renderer renderer;

	bool multiplayer = false;

	float SCR_WIDTH = 1360;
	float SCR_HEIGHT = 960;

	UIToolbar menu;
	UICrosshair crosshair;
	OutlineBlock outline;
	Skybox skybox;

	player_s player;
	chunk_map_t chunks;

	memory_arena block_arena;
	memory_arena noise_arena;

	float delta_time = 0.0f;	// time between current frame and last frame
	float last_frame = 0.0f;

	int texture_slot_counter = 0;

	glm::mat4 perspective;
	glm::mat4 orthographic;
} GameState;

GLFWwindow* init_and_create_window();

void state_allocate_memory()
{
	const auto block_arena_size = (sizeof(Block) * BLOCKS_IN_CHUNK) * CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE;
	const auto noise_arena_size = sizeof(float) * BLOCKS_IN_CHUNK * CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE;

	const int mb = 1024 * 1024;
	g_logger_info("Allocating: %dMB", block_arena_size / mb);
	g_logger_info("Allocating: %dMB", noise_arena_size / mb);

	memory_arena_init(&GameState.block_arena, block_arena_size, sizeof(Block) * BLOCKS_IN_CHUNK);
	memory_arena_init(&GameState.noise_arena, noise_arena_size, sizeof(float) * (CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT * CHUNK_SIZE_WIDTH));
}

void state_global_init()
{
	// Start 5 blocks over the ground
	glm::ivec3 camera_pos = glm::vec3(0.0f, 50, 0.0f);
	//glm::ivec3 camera_pos = glm::vec3(0.0f, 127.0f, 0.0f);
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

	platform_sound_init();

	GameState.menu = menu_create(GameState.SCR_WIDTH, GameState.SCR_HEIGHT);
	g_logger_debug("Menu created");
	GameState.crosshair = crosshair_create(GameState.SCR_WIDTH, GameState.SCR_HEIGHT);
	g_logger_debug("Crosshair created");

	state_allocate_memory();
}

void create_and_init_chunk(const int x, const int z)
{
	Chunk chunk;

	chunk.blocks = (Block*)memory_arena_get(&GameState.block_arena);
	chunk.chunks = &GameState.chunks;
	chunk.initialized = false;
	chunk.needs_light_recalc = true;
	chunk.needs_remesh = true;
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

	g_logger_debug("Chunk size width: %d - height: %d", CHUNK_SIZE_WIDTH, CHUNK_SIZE_HEIGHT);

	for (auto& iter : GameState.chunks)
	{
		set_chunk_neighbors(&iter.second);
	}
	// Emscripten dosent like this and i cba doing it another way
#define NOMULTI
#if !defined(__EMSCRIPTEN) && !defined(NOMULTI)
	std::for_each(std::execution::par_unseq, std::begin(GameState.chunks), std::end(GameState.chunks),
		[&](auto& iter)
	{
		world_generate(iter.second.blocks, nullptr, iter.first.x, iter.first.y, CHUNK_SIZE_WIDTH, CHUNK_SIZE_HEIGHT);
	});
#else
	for (auto& iter : GameState.chunks)
	{
		world_generate(iter.second.blocks, nullptr, iter.first.x, iter.first.y, CHUNK_SIZE_WIDTH, CHUNK_SIZE_HEIGHT);
	}
#endif // !__EMSCRIPTEN__
	for (auto& iter : GameState.chunks)
	{
		LightsInWorld.push_back({ iter.first, 1344 });
	}
	g_logger_info("Done initializing game world!");
}

BlockType inventory[9] =
{
	BlockType::STONE,
	BlockType::DIRT,
	BlockType::DIRT_GRASS,
	BlockType::GLOWSTONE,
	BlockType::BRICKS,
	BlockType::GLASS,
	BlockType::LEAVES,
	BlockType::GRASS,
	BlockType::WATER,
};

void play_block_sound(BlockType type, bool remove)
{
	char block_name[BLOCK_NAME_MAX_SIZE];
	block_get_sound(type, remove, block_name, sizeof(block_name));

	char file_name[100];
	snprintf(file_name, sizeof(file_name), "%s.oga", block_name);
	file_name[sizeof(file_name) - 1] = '\0';

	platform_sound_play(file_name);
}

// TODO: Cleanup this disgusting method
void handle_block_hit(ray_hit_result ray_hit, bool remove)
{
	if (ray_hit.chunk_hit == nullptr)
		return;

	g_logger_debug("RAY HIT: Local(%d,%d,%d) ChunkPos(%d,%d)",
		ray_hit.block_pos.x, ray_hit.block_pos.y, ray_hit.block_pos.z,
		ray_hit.chunk_hit->world_pos.x, ray_hit.chunk_hit->world_pos.y);

	BlockType type = inventory[GameState.menu.item_selected];

	if (ray_hit.top_half)
	{
		//g_logger_debug("TOP HALF");
	}

	// what chunk and block to process
	Chunk* hit_chunk = nullptr;
	glm::ivec3 b_pos = ray_hit.block_pos;

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
		play_block_sound(type, remove);
		chunk_set_block(hit_chunk, b_pos, type);
	}
	else
	{
		if (remove)
		{
			//auto asd = get_blocks_in_circle(hit_chunk, b_pos, 4);
			//for (auto b : asd)
			//{
			//	b->type = BlockType::AIR;
			//	//b.c->dirty = true;
			//}
			play_block_sound(chunk_get_block(hit_chunk, b_pos).b->type, remove);
			chunk_set_block(hit_chunk, b_pos, BlockType::AIR);
		}
		else
		{
			chunk_set_block(hit_chunk, b_pos, type);
			if (type != BlockType::AIR)
				play_block_sound(type, remove);
		}

		hit_chunk->verts_in_use = 0;
		hit_chunk->verts_in_use_transparent = 0;
	}
}

// TODO: Move to renderer
void opengl_clear_screen()
{
}

bool is_pos_inside_block(glm::vec2 pos)
{
	// TODO: Filter chunks
	for (auto& c : GameState.chunks)
	{
		for (auto& p : c.second.gpu_data_opaque)
		{
			if (GameState.player.camera.Position.x <= p.x && GameState.player.camera.Position.x + 1 >= p.x
				&& GameState.player.camera.Position.z <= p.z && GameState.player.camera.Position.z + 1 >= p.z)
			{
				if (GameState.player.camera.Position.y > p.y + 1)
				{
					return true;
				}
			}
		}
	}

	return false;
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
	{
		// Changes player velocity aswell
		GameState.player.camera.MoveWithVelocity(FORWARD, delta_time);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		GameState.player.camera.MoveWithVelocity(Camera_Movement::BACKWARD, delta_time);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		GameState.player.camera.MoveWithVelocity(Camera_Movement::LEFT, delta_time);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		GameState.player.camera.MoveWithVelocity(Camera_Movement::RIGHT, delta_time);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		GameState.player.camera.MoveWithVelocity(Camera_Movement::DOWN, delta_time);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		GameState.player.camera.MoveWithVelocity(Camera_Movement::UP, delta_time);
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		menu_select_item(&GameState.menu, 0);
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		menu_select_item(&GameState.menu, 1);
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		menu_select_item(&GameState.menu, 2);
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		menu_select_item(&GameState.menu, 3);
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		menu_select_item(&GameState.menu, 4);
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
		menu_select_item(&GameState.menu, 5);
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
		menu_select_item(&GameState.menu, 6);
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
		menu_select_item(&GameState.menu, 7);
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
		menu_select_item(&GameState.menu, 8);
}

void render_3d_vegetation()
{
	glm::mat4 view = GameState.player.camera.GetViewMatrix();
	for (auto& iter : GameState.chunks)
	{
		glUniform3f(GameState.renderer.shaders[ShaderType::SHADER_CHUNK].uniform_locations[3], GameState.player.camera.Position.x, GameState.player.camera.Position.y, GameState.player.camera.Position.z);
		glm::vec3 position = glm::vec3(iter.first.x, 0, iter.first.y);
		chunk_render_veg(&iter.second, &GameState.renderer, view, position);
	}
}

void render_3d_transparent()
{
	glm::mat4 view = GameState.player.camera.GetViewMatrix();

	std::vector<std::pair<glm::ivec2, Chunk*>> chunks_vector;
	for (auto& pair : GameState.chunks)
	{
		chunks_vector.push_back({ pair.first, &pair.second });
	}

	for (auto& iter : chunks_vector)
	{
		glUniform3f(GameState.renderer.shaders[ShaderType::SHADER_CHUNK].uniform_locations[3], GameState.player.camera.Position.x, GameState.player.camera.Position.y, GameState.player.camera.Position.z);
		glm::vec3 position = glm::vec3(iter.first.x, 0, iter.first.y);
		chunk_render_transparent(iter.second, &GameState.renderer, view, position);
	}
}

void render_3d_opaque()
{
	glm::mat4 view = GameState.player.camera.GetViewMatrix();

	// Render the opaque objects first
	for (auto& iter : GameState.chunks)
	{
		glm::vec3 position = glm::vec3(iter.first.x, 0, iter.first.y);
		chunk_render_opaque(&iter.second, &GameState.renderer, view, position);
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
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render skybox
	glDepthMask(GL_FALSE);    // Disable depth writing
	skybox_render(&GameState.skybox, &GameState.renderer, GameState.player.camera.GetViewMatrix());
	glDepthMask(GL_TRUE);

	// Render opaque
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	render_3d_opaque();

	// TODO: SORT SEMI-TRANSPARENT OBJECTS
	// Render semi-transparent
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	render_3d_vegetation();
	glDepthMask(GL_TRUE);

	// TODO: SORT TRANSPARENT

	// Render transparent
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	render_3d_transparent();
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	outline_render(&GameState.outline, &GameState.renderer, GameState.player.camera.GetViewMatrix());
	// Render UI
	glDisable(GL_DEPTH_TEST); // Disable depth testing
	glEnable(GL_BLEND);       // Enable blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Typical alpha blending
	render_2d();
	glDisable(GL_BLEND);      // Disable blending
}

void game_update(float deltaTime)
{
	const float projection_fov = glm::radians(GameState.player.camera.Zoom);
	const float projection_aspect = GameState.SCR_WIDTH / GameState.SCR_HEIGHT;

	GameState.perspective = glm::perspective(projection_fov, projection_aspect, 0.1f, 2000.0f);
	GameState.orthographic = glm::ortho(0.0f, GameState.SCR_WIDTH, 0.0f, GameState.SCR_HEIGHT, -100.0f, 100.0f);

	GameState.player.camera.Update(deltaTime);

	renderer_update(&GameState.renderer, GameState.perspective, GameState.orthographic);
	outline_update(&GameState.outline, GameState.player.camera.Position, GameState.player.camera.Front, &GameState.chunks);
	chunk_update(&GameState.chunks, GameState.player.camera.Position);
}

#pragma comment (lib, "winmm.lib")
bool hasJoined = false;
void game_main_loop()
{
	static double lastFrameTime = glfwGetTime();
	static double lastFPSTime = glfwGetTime();
	static int numberOfFrames = 0;

	// Calculate delta time
	double currentTime = glfwGetTime();
	float delta_time = currentTime - lastFrameTime;
	lastFrameTime = currentTime;

	// Increment frame counter
	numberOfFrames++;

	// Check if a second has passed; if so, print FPS
	if (currentTime - lastFPSTime >= 1.0)
	{
		//std::cout << 1000.0 / double(numberOfFrames) << " ms/frame (" << double(numberOfFrames) << " fps)" << std::endl;
		numberOfFrames = 0;
		lastFPSTime += 1.0;
	}
	glfwPollEvents();

	processInput(GameState.window, delta_time);

	game_update(delta_time);

	game_render();

	glfwSwapBuffers(GameState.window);
}

// Steps should ideally be calculated like this:
// std::max(1.0, std::min(std::floor(width / 320.0), std::floor(height / 240.0)));
// but who in their right mind uses >1920
static glm::ivec2 scale_res[] =
{
	glm::vec2(640, 480),
	glm::vec2(960, 720),
	glm::vec2(1280, 960),
};

void update_viewport()
{
	const int width = GameState.SCR_WIDTH;
	const int height = GameState.SCR_HEIGHT;
	const float near = 0.1f;
	const float far = 1000.0f;
	// Update perspective projection for 3D rendering
	GameState.perspective = glm::perspective(GameState.player.camera.Zoom, (float)width / (float)height, near, far);
	// Update orthographic projection for GUI rendering
	GameState.orthographic = glm::ortho(0.0f, (float)width, 0.0f, (float)height);

	glViewport(0, 0, width, height);

	int scale = 1;
	for (int i = 0; i < 3; i++)
	{
		if (scale_res[i].x < width && scale_res[i].y < height)
		{
			scale++;
		}
	}

	menu_scale(&GameState.menu, GameState.SCR_WIDTH, GameState.SCR_HEIGHT, scale);
}


int main()
{
	// Global configurations
	srand(time(NULL));
	std::ios_base::sync_with_stdio(false);

	state_global_init();

	init_game_world();

	memory_arena_dealloc(&GameState.noise_arena);

	update_viewport();

	platform_set_main_loop(GameState.window, game_main_loop);

	return 0;
}

glm::dvec2 last_x;
void on_scroll(PlatformScrollDirection direction)
{
	double step = 1;
	if (direction == PlatformScrollDirection::SCROLL_UP)
	{
		if (GameState.menu.item_selected != 8)
		{
			menu_select_prev(&GameState.menu);
		}
	}
	else if (direction == PlatformScrollDirection::SCROLL_DOWN)
	{
		if (GameState.menu.item_selected != 0)
		{
			menu_select_next(&GameState.menu);
		}
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	GameState.SCR_WIDTH = width;
	GameState.SCR_HEIGHT = height;

	update_viewport();
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

	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	platform_post_setup(window);
	platform_set_scroll_callback((void*)window, on_scroll);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	framebuffer_size_callback(window, GameState.SCR_WIDTH, GameState.SCR_HEIGHT);

	g_logger_info("OpenGL version: %s", glGetString(GL_VERSION));
	return window;
}
