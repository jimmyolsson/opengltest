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

const int SHADER_COUNT = 3;

struct State
{
	GLFWwindow* window;

	const unsigned int SCR_WIDTH = 1360;
	const unsigned int SCR_HEIGHT = 960;

	// TODO: UI
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

const int WORLD_GEN_HEIGHT = CHUNK_SIZE_HEIGHT;
const int WORLD_GEN_WIDTH = CHUNK_SIZE_WIDTH;

#include <FastNoise/FastNoise.h>
FastNoise::SmartNode<> asdnoise = FastNoise::NewFromEncodedNodeTree("EQACAAAAAAAgQBAAAAAAQBkAEwDD9Sg/DQAEAAAAAAAgQAkAAGZmJj8AAAAAPwEEAAAAAAAAAEBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAM3MTD4AMzMzPwAAAAA/");

struct structure_node
{
	glm::ivec3 pos;
	block_type type;
};
std::vector<structure_node> tree_structure = {
	{{0, 1, 0}, block_type::OAK_LOG},
	{{0, 2, 0}, block_type::OAK_LOG},
	{{0, 3, 0}, block_type::OAK_LOG},
	{{0, 4, 0}, block_type::OAK_LOG},
	{{0, 5, 0}, block_type::OAK_LOG},
	{{0, 6, 0}, block_type::OAK_LOG},

	{{-2, 4, 1}, block_type::LEAVES},
	{{-1, 4, 1}, block_type::LEAVES},
	{{1, 4, 1}, block_type::LEAVES},
	{{2, 4, 1}, block_type::LEAVES},

	{{-2, 4, 0}, block_type::LEAVES},
	{{-1, 4, 0}, block_type::LEAVES},
	{{1, 4, 0}, block_type::LEAVES},
	{{2, 4, 0}, block_type::LEAVES},

	{{-2, 4, -1}, block_type::LEAVES},
	{{-1, 4, -1}, block_type::LEAVES},
	{{1, 4, -1}, block_type::LEAVES},
	{{2, 4, -1}, block_type::LEAVES},

	{{1, 4, -2}, block_type::LEAVES},
	{{1, 4, -1}, block_type::LEAVES},
	{{1, 4, 1}, block_type::LEAVES},
	{{1, 4, 2}, block_type::LEAVES},

	{{0, 4, -2}, block_type::LEAVES},
	{{0, 4, -1}, block_type::LEAVES},
	{{0, 4, 1}, block_type::LEAVES},
	{{0, 4, 2}, block_type::LEAVES},

	{{-1, 4, -2}, block_type::LEAVES},
	{{-1, 4, -1}, block_type::LEAVES},
	{{-1, 4, 1}, block_type::LEAVES},
	{{-1, 4, 2}, block_type::LEAVES},

	{{-2, 5, 1}, block_type::LEAVES},
	{{-1, 5, 1}, block_type::LEAVES},
	{{1, 5, 1}, block_type::LEAVES},
	{{2, 5, 1}, block_type::LEAVES},

	{{-2, 5, 0}, block_type::LEAVES},
	{{-1, 5, 0}, block_type::LEAVES},
	{{1, 5, 0}, block_type::LEAVES},
	{{2, 5, 0}, block_type::LEAVES},

	{{-2, 5, -1}, block_type::LEAVES},
	{{-1, 5, -1}, block_type::LEAVES},
	{{1, 5, -1}, block_type::LEAVES},
	{{2, 5, -1}, block_type::LEAVES},

	{{1, 5, -2}, block_type::LEAVES},
	{{1, 5, -1}, block_type::LEAVES},
	{{1, 5, 1}, block_type::LEAVES},
	{{1, 5, 2}, block_type::LEAVES},

	{{0, 5, -2}, block_type::LEAVES},
	{{0, 5, -1}, block_type::LEAVES},
	{{0, 5, 1}, block_type::LEAVES},
	{{0, 5, 2}, block_type::LEAVES},

	{{-1, 5, -2}, block_type::LEAVES},
	{{-1, 5, -1}, block_type::LEAVES},
	{{-1, 5, 1}, block_type::LEAVES},
	{{-1, 5, 2}, block_type::LEAVES},

	{{-1, 6, 1}, block_type::LEAVES},
	{{1, 6, 1}, block_type::LEAVES},

	{{-1, 6, 0}, block_type::LEAVES},
	{{1, 6, 0}, block_type::LEAVES},

	{{-1, 6, -1}, block_type::LEAVES},

	{{1, 6, -1}, block_type::LEAVES},

	{{1, 6, -1}, block_type::LEAVES},
	{{1, 6, 1}, block_type::LEAVES},

	{{0, 6, -1}, block_type::LEAVES},
	{{0, 6, 1}, block_type::LEAVES},

	{{-1, 6, -1}, block_type::LEAVES},
	{{-1, 6, 1}, block_type::LEAVES},

	{{0, 7, 0}, block_type::LEAVES},
	{{-1, 7, 0}, block_type::LEAVES},
	{{1, 7, 0}, block_type::LEAVES},
	{{0, 7, -1}, block_type::LEAVES},
	{{0, 7, 1}, block_type::LEAVES},
};

void place_tree(chunk* c, glm::ivec3 spawn_point_pos, std::vector<structure_node> n)
{
	// TODO: bounds check
	//std::cout << "SPHERE::size: " << n.size() << '\n';
	for (int i = 0; i < n.size(); i++)
	{
		auto& node = n[i];
		glm::ivec3 pos = spawn_point_pos + node.pos;
		chunk_set_block(c, pos, node.type);
	}
}
static int to_1d_array(short x, short y, short z)
{
	return (z * WORLD_GEN_WIDTH * WORLD_GEN_HEIGHT) + (y * WORLD_GEN_WIDTH) + x;
}

void generate_world_noise(block* blocks, const int xoffset, const int zoffset)
{
	const float frequency = 0.002f;
	const float threshold = 0.01f;

	float* noise = (float*)memory_arena_get(&GameState.noise_arena, sizeof(float) * (WORLD_GEN_WIDTH * WORLD_GEN_HEIGHT * WORLD_GEN_WIDTH));

	auto min_max = asdnoise.get()->GenUniformGrid3D(noise, xoffset, -WORLD_GEN_HEIGHT / 2, zoffset, WORLD_GEN_WIDTH, WORLD_GEN_HEIGHT, WORLD_GEN_WIDTH, frequency, 1337);

	for (int i = 0; i < WORLD_GEN_WIDTH * WORLD_GEN_HEIGHT * WORLD_GEN_WIDTH; i++)
		noise[i] *= -1;

	const static int sea_level = 130;
	for (int z = 0; z < WORLD_GEN_WIDTH; z++)
	{
		for (int y = 0; y < WORLD_GEN_HEIGHT; y++)
		{
			for (int x = 0; x < WORLD_GEN_WIDTH; x++)
			{
				int index = to_1d_array(x, y, z);
				if (noise[index] > threshold)
				{
					if (noise[to_1d_array(x, y + 1, z)] > threshold)
					{
						blocks[index].type = block_type::DIRT;
					}
					else
					{
						blocks[index].type = block_type::DIRT_GRASS;
					}
				}
			}
		}
	}
	//for (int z = 0; z < WORLD_GEN_WIDTH; z++)
	//{
	//	for (int y = 0; y < WORLD_GEN_HEIGHT; y++)
	//	{
	//		for (int x = 0; x < WORLD_GEN_WIDTH; x++)
	//		{
	//			auto& curr_c = chunk->blocks[to_1d_array(x, y, z)];
	//			if (y < sea_level && curr_c.type == block_type::AIR)
	//			{
	//				curr_c.type = block_type::WATER;
	//			}
	//		}
	//	}
	//}
}

void generate_world_flatgrass(block* blocks, const int xoffset, const int zoffset)
{
	for (int z = 0; z < WORLD_GEN_WIDTH; z++)
	{
		for (int y = 0; y < WORLD_GEN_HEIGHT; y++)
		{
			for (int x = 0; x < WORLD_GEN_WIDTH; x++)
			{
				int index = to_1d_array(x, y, z);
				if (y < WORLD_GEN_HEIGHT / 6)
				{
					blocks[index].type = block_type::DIRT_GRASS;
				}
				if (y < (WORLD_GEN_HEIGHT / 6) - 1)
				{
					blocks[index].type = block_type::DIRT_GRASS;
				}
			}
		}
	}
}

void generate_world(block* blocks, const int xoffset, const int zoffset)
{
#if _DEBUG
	// dosent work in debug otherwise..
	for (int z = 0; z < WORLD_GEN_WIDTH; z++)
	{
		for (int y = 0; y < WORLD_GEN_HEIGHT; y++)
		{
			for (int x = 0; x < WORLD_GEN_WIDTH; x++)
			{
				int index = to_1d_array(x, y, z);
				blocks[index].type = block_type::AIR;
				blocks[index].sky = false;
			}
		}
	}
#endif // DEBUG

	//generate_world_flatgrass(blocks, xoffset, zoffset);
	generate_world_noise(blocks, xoffset, zoffset);
}

void state_allocate_memory_arenas()
{
	auto block_arena_size = (sizeof(block) * BLOCKS_IN_CHUNK) * CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE;
	auto noise_arena_size = sizeof(float) * ((CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT * CHUNK_SIZE_WIDTH) * CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE);
	auto size_chunk = sizeof(block_size_t) * ((CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT * CHUNK_SIZE_WIDTH) * CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE);
	std::cout << "block: " << block_arena_size << "bytes\n";
	std::cout << "noise: " << noise_arena_size << "bytes\n";
	std::cout << "chunk: " << size_chunk << "bytes\n";

	memory_arena_init(&GameState.block_arena, block_arena_size);
	memory_arena_init(&GameState.noise_arena, noise_arena_size);
	memory_arena_init(&GameState.chunk_arena, size_chunk);
}

void state_global_init()
{
	GameState.player.camera = Camera(glm::vec3(0.0f, 50.0f, 0.0f));
	GameState.window = init_and_create_window();
	GameState.chunks = {};

	GameState.crosshair = crosshair_create();
	GameState.outline = outline_create();

	GameState.delta_time = 0;
	GameState.last_frame = 0;

	sound_init(&GameState.sound_manager);

	state_allocate_memory_arenas();
}

void create_and_init_chunk(const int x, const int z)
{
	chunk chunk;

	chunk.blocks = (block*)memory_arena_get(&GameState.block_arena, sizeof(block) * BLOCKS_IN_CHUNK);
	chunk.gpu_data_arr = (block_size_t*)memory_arena_get(&GameState.chunk_arena, sizeof(block_size_t) * BLOCKS_IN_CHUNK);
	chunk.chunks = &GameState.chunks;
	chunk.initialized = false;
	glm::ivec2 pos = glm::vec2(x * CHUNK_SIZE_WIDTH, z * CHUNK_SIZE_WIDTH);
	chunk.world_pos = pos;

	generate_world(chunk.blocks, pos.x, pos.y);

	GameState.chunks[pos] = chunk;
}

std::vector<structure_node> sphere;
// Function to put pixels
// at subsequence points
void drawCircle(int xc, int yc, int x, int y, int world_y, std::vector<structure_node>& sphere)
{
	sphere.push_back({ { xc + x, world_y, yc + y }, block_type::STONE });
	sphere.push_back({ { xc - x, world_y, yc + y }, block_type::STONE });
	sphere.push_back({ { xc + x, world_y, yc - y }, block_type::STONE });
	sphere.push_back({ { xc - x, world_y, yc - y }, block_type::STONE });
	sphere.push_back({ { xc + y, world_y, yc + x }, block_type::STONE });
	sphere.push_back({ { xc - y, world_y, yc + x }, block_type::STONE });
	sphere.push_back({ { xc + y, world_y, yc - x }, block_type::STONE });
	sphere.push_back({ { xc - y, world_y, yc - x }, block_type::STONE });
}

// Function for circle-generation
// using Bresenham's algorithm
void circleBres(int xc, int yc, int r, int world_y, std::vector<structure_node>& sphere)
{
	int x = 0, y = r;
	int d = 3 - 2 * r;
	drawCircle(xc, yc, x, y, world_y, sphere);
	while (y >= x)
	{
		// for each pixel we will
		// draw all eight pixels

		x++;

		// check for decision parameter
		// and correspondingly
		// update d, x, y
		if (d > 0)
		{
			y--;
			d = d + 4 * (x - y) + 10;
		}
		else
			d = d + 4 * x + 6;
		drawCircle(xc, yc, x, y, world_y, sphere);
	}
}

const int sphere_size = 16;
int map[sphere_size][sphere_size][sphere_size];

std::vector<structure_node> sphere_algo(int x0, int y0, int z0, int r)
{
	int col = 1;
	int n = sphere_size;
	for (int x = 0; x < sphere_size; x++)
		for (int y = 0; y < sphere_size; y++)
			for (int z = 0; z < sphere_size; z++)
			{
				map[x][y][z] = 0;
			}

	int x, y, z, xa, ya, za, xb, yb, zb, xr, yr, zr, xx, yy, zz, rr = r * r;
	// bounding box
	xa = x0 - r; if (xa < 0) xa = 0; xb = x0 + r; if (xb > n) xb = n;
	ya = y0 - r; if (ya < 0) ya = 0; yb = y0 + r; if (yb > n) yb = n;
	za = z0 - r; if (za < 0) za = 0; zb = z0 + r; if (zb > n) zb = n;
	// project xy plane
	for (x = xa, xr = x - x0, xx = xr * xr; x < xb; x++, xr++, xx = xr * xr)
		for (y = ya, yr = y - y0, yy = yr * yr; y < yb; y++, yr++, yy = yr * yr)
		{
			zz = rr - xx - yy; if (zz < 0) continue; zr = sqrt(zz);
			z = z0 - zr; if ((z > 0) && (z < n)) map[x][y][z] = col;
			z = z0 + zr; if ((z > 0) && (z < n)) map[x][y][z] = col;
		}
	// project xz plane
	for (x = xa, xr = x - x0, xx = xr * xr; x < xb; x++, xr++, xx = xr * xr)
		for (z = za, zr = z - z0, zz = zr * zr; z < zb; z++, zr++, zz = zr * zr)
		{
			yy = rr - xx - zz; if (yy < 0) continue; yr = sqrt(yy);
			y = y0 - yr; if ((y > 0) && (y < n)) map[x][y][z] = col;
			y = y0 + yr; if ((y > 0) && (y < n)) map[x][y][z] = col;
		}
	// project yz plane
	for (y = ya, yr = y - y0, yy = yr * yr; y < yb; y++, yr++, yy = yr * yr)
		for (z = za, zr = z - z0, zz = zr * zr; z < zb; z++, zr++, zz = zr * zr)
		{
			xx = rr - zz - yy; if (xx < 0) continue; xr = sqrt(xx);
			x = x0 - xr; if ((x > 0) && (x < n)) map[x][y][z] = col;
			x = x0 + xr; if ((x > 0) && (x < n)) map[x][y][z] = col;
		}

	std::vector<structure_node> a;
	for (int x = 0; x < sphere_size; x++)
		for (int y = 0; y < sphere_size; y++)
			for (int z = 0; z < sphere_size; z++)
			{
				if (map[x][y][z] != 0)
					a.push_back({ {x, y, z},{block_type::STONE} });
			}
	return a;
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

	auto a = sphere_algo(5, 10, 5, 2);

	for (auto& iter : GameState.chunks)
	{
		place_tree(&iter.second, glm::ivec3(CHUNK_SIZE_WIDTH / 2, 41, CHUNK_SIZE_WIDTH / 2), a);
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

	chunk* hit_chunk;
	//block* hit_block = chunk_get_block(hit_chunk, ray_hit.block_pos + ray_hit.direction);

	bool another_chunk = false;
	if (ray_hit.block_pos.x >= CHUNK_SIZE_WIDTH)
	{
		hit_chunk = ray_hit.chunk_hit->right_neighbor;
		another_chunk = true;
	}
	else
	{
		hit_chunk = ray_hit.chunk_hit;
	}

	using namespace std::chrono;
	//if (another_chunk)
	//{
	//	auto a = steady_clock::now();
	//	b_pos = glm::vec3((ray_hit.block_pos.x - CHUNK_SIZE_WIDTH - 1), ray_hit.block_pos.y, ray_hit.block_pos.z);

	//	if (!remove)
	//		b_pos += ray_hit.direction;

	//	GameState.chunks[ray_hit.chunk_hit->right_neighbor->world_pos].blocks[to_1d_array(b_pos)].type = type;

	//	auto aa = steady_clock::now();
	//	auto aaa = duration_cast<milliseconds>(aa - a).count();
	//	std::cout << "modify(another chunk): " << aaa << "ms\n";

	//	chunk* c = &GameState.chunks[ray_hit.chunk_hit->right_neighbor->world_pos];
	//	ChunkPrivate::generate_mesh(c, ray_hit.chunk_hit->right_neighbor->world_pos);
	//	glDeleteBuffers(1, &c->vbo_handle);
	//	glDeleteVertexArrays(1, &c->vao_handle);
	//	ChunkPrivate::init_buffers(c);
	//}
	//else
	{
		glm::ivec3 b_pos = ray_hit.block_pos;

		if (!remove)
			b_pos += ray_hit.direction;

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

		//??
		hit_chunk->blocks_in_use = 0;
		hit_chunk->dirty = true;
	}
}

void chunk_init(const int x, const int z)
{
	chunk c;
	c.blocks = (block*)memory_arena_get(&GameState.block_arena, sizeof(block) * BLOCKS_IN_CHUNK);
	c.gpu_data_arr = (block_size_t*)memory_arena_get(&GameState.chunk_arena, sizeof(block_size_t) * BLOCKS_IN_CHUNK);
	c.chunks = &GameState.chunks;
	c.initialized = false;
	glm::ivec2 pos = glm::vec2(x * CHUNK_SIZE_WIDTH, z * CHUNK_SIZE_WIDTH);
	c.world_pos = pos;

	generate_world(c.blocks, pos.x, pos.y);

	GameState.chunks[pos] = c;
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
	const glm::mat4 projection = glm::perspective(glm::radians(GameState.player.camera.Zoom), (float)GameState.SCR_WIDTH / (float)GameState.SCR_HEIGHT, 0.1f, 1000.0f);
	const glm::mat4 view = GameState.player.camera.GetViewMatrix();
	const glm::mat4 model = glm::mat4(1.0f);

	lightingShader->use();

	lightingShader->setMat4("projection", projection);
	lightingShader->setMat4("view", view);
	lightingShader->setMat4("model", model);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, atlas);

	for (auto& iter : GameState.chunks)
	{
		//std::cout << "game_render::chunks\n";
		lightingShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(iter.first.x, 0, iter.first.y)));
		chunk_render(iter.second);
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	outline_render(&GameState.outline, projection, view);
	crosshair_render(&GameState.crosshair, GameState.SCR_WIDTH, GameState.SCR_HEIGHT);
}

void game_update()
{
	outline_update(&GameState.outline, GameState.player.camera.Position, GameState.player.camera.Front, &GameState.chunks);
	chunk_update(&GameState.chunks);
}

void game_main_loop(unsigned int atlas)
{
	Shader lightingShader("..\\resources\\shaders\\opaque_world.shadervs", "..\\resources\\shaders\\opaque_world.shaderfs");

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
	set_opengl_constants();

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
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
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
	//stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
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
	unsigned char* water = load_png("..\\resources\\textures\\water.png");
	unsigned char* leaves = load_png("..\\resources\\textures\\leaves.png");
	unsigned char* oak_log = load_png("..\\resources\\textures\\oak_log.png");
	unsigned char* oak_log_top = load_png("..\\resources\\textures\\oak_log_top.png");

	unsigned int atlas;
	GLsizei width = 16;
	GLsizei height = 16;

	// CURRENT NUMBER OF TEXTURES
	GLsizei layerCount = 9;
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
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 5, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, water);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 6, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, leaves);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 7, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, oak_log);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 8, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, oak_log_top);

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