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
#include "GLFW/glfw3.h"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image/stb_image.h"

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

#include "shader_m.h"
//#include "shader.h"
#include "camera.h"
#include "chunk.h"
#include "blocks/block.h"
#include "memory_arena.h"
#include "crosshair.h"

#include <iostream>
#include <tuple>
#include <algorithm>
#include <execution>
#include <cmath>
#include <array>
#include <chrono>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
GLFWwindow* init_and_create_window();
std::tuple<GLuint, GLuint> load_textures();
void set_opengl_constants();

// settings
constexpr unsigned int SCR_WIDTH = 1360;
constexpr unsigned int SCR_HEIGHT = 960;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


const int WORLD_GEN_HEIGHT = CHUNK_SIZE_HEIGHT;
const int WORLD_GEN_WIDTH = CHUNK_SIZE_WIDTH;

//#define FLATGRASS

#include "robin_hood.h"
// camera
//Camera camera(glm::vec3(190.0f, 178.0f, -112.0f));
Camera camera(glm::vec3(0.0f, 60.0f, 0.0f));

memory_arena block_arena;
memory_arena noise_arena;
memory_arena chunk_arena;

static robin_hood::unordered_flat_map<glm::ivec2, chunk> chunks = {};

static int counterr = 0;

#include <FastNoise/FastNoise.h>
FastNoise::SmartNode<> asdnoise = FastNoise::NewFromEncodedNodeTree("EQACAAAAAAAgQBAAAAAAQBkAEwDD9Sg/DQAEAAAAAAAgQAkAAGZmJj8AAAAAPwEEAAAAAAAAAEBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAM3MTD4AMzMzPwAAAAA/");
static int to_1d_array(glm::ivec3 pos)
{
	return (pos.z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (pos.y * CHUNK_SIZE_WIDTH) + pos.x;
}
static int to_1d_array(int x, int y, int z)
{
	return (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x;
}

struct structure_node {
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

void place_tree(chunk* c, glm::ivec3 spawn_point_pos)
{
	// TODO: bounds check
	for (int i = 0; i < tree_structure.size(); i++)
	{
		auto& node = tree_structure[i];
		glm::ivec3 pos = spawn_point_pos + node.pos;
		c->blocks[to_1d_array(pos)].type = node.type;
	}
}


void generate_world_noise(const chunk* chunk, const int xoffset, const int zoffset)
{
	const float frequency = 0.002f;
	const float threshold = 0.02f;

	float* noise = (float*)memory_arena_get(&noise_arena, sizeof(float) * (WORLD_GEN_WIDTH * WORLD_GEN_HEIGHT * WORLD_GEN_WIDTH));

	auto min_max = asdnoise.get()->GenUniformGrid3D(noise, xoffset, -WORLD_GEN_HEIGHT / 2, zoffset, WORLD_GEN_WIDTH, WORLD_GEN_HEIGHT, WORLD_GEN_WIDTH, frequency, 1337);

	for (int i = 0; i < WORLD_GEN_WIDTH * WORLD_GEN_HEIGHT * WORLD_GEN_WIDTH; i++)
		noise[i] *= -1;

	const int sea_level = 50;
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
						chunk->blocks[index].type = block_type::DIRT;
					}
					else
					{
						chunk->blocks[index].type = block_type::DIRT_GRASS;
					}
				}
			}
		}
	}
}

void generate_world_flatgrass(const chunk* chunk, const int xoffset, const int zoffset)
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
					chunk->blocks[index].type = block_type::DIRT_GRASS;
				}
				if (y < (WORLD_GEN_HEIGHT / 6) - 1)
				{
					chunk->blocks[index].type = block_type::DIRT_GRASS;
				}
			}
		}
	}
}

void generate_world(const chunk* chunk, const int xoffset, const int zoffset)
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
				chunk->blocks[index].type = block_type::AIR;
				chunk->blocks[index].sky = false;
			}
		}
	}
#endif // DEBUG

	//generate_world_flatgrass(chunk, xoffset, zoffset);
	generate_world_noise(chunk, xoffset, zoffset);
}

void create_and_init_chunk(const int x, const int z)
{
	chunk chunk;

	chunk.blocks = (block*)memory_arena_get(&block_arena, sizeof(block) * BLOCKS_IN_CHUNK);
	chunk.gpu_data_arr = (block_size_t*)memory_arena_get(&chunk_arena, sizeof(block_size_t) * BLOCKS_IN_CHUNK);
	chunk.chunks = &chunks;
	chunk.initialized = false;
	glm::ivec2 pos = glm::vec2(x * CHUNK_SIZE_WIDTH, z * CHUNK_SIZE_WIDTH);
	chunk.world_pos = pos;

	generate_world(&chunk, pos.x, pos.y);

	chunks[pos] = chunk;
}

void init_chunks()
{
	using namespace std::chrono;

	auto start_noise_gen = steady_clock::now();
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

	for (auto& iter : chunks)
	{
		place_tree(&iter.second, glm::ivec3(CHUNK_SIZE_WIDTH / 2, 41, CHUNK_SIZE_WIDTH / 2));
	}

	// ----------------- MESH GEN -----------------
	auto start_meshgen = std::chrono::steady_clock::now();
	std::for_each(std::execution::par_unseq, std::begin(chunks), std::end(chunks),
		[&](auto& iter)
		{
			ChunkPrivate::generate_mesh(&iter.second, iter.first);
		});

	// ----------------- INIT BUFFERS -----------------
	for (auto& iter : chunks)
	{
		ChunkPrivate::init_buffers(&iter.second);
	}
}
struct outline_block {
	unsigned int vao;
	unsigned int vbo;
	glm::ivec3 position;
} outline_b;

void create_outline()
{
	const int vert_count = 30;
	std::vector<int> gpu_data;
	{
		int index = 0;
		for (int i = 0; i < 6; i++)
		{
			gpu_data.push_back(ChunkPrivate::m_back_verticies[index]);
			gpu_data.push_back(ChunkPrivate::m_back_verticies[index + 1]);
			gpu_data.push_back(ChunkPrivate::m_back_verticies[index + 2]);
			gpu_data.push_back(ChunkPrivate::m_back_verticies[index + 3]);
			gpu_data.push_back(ChunkPrivate::m_back_verticies[index + 4]);
			index += 5;
		}
	}
	{
		int index = 0;
		for (int i = 0; i < 6; i++)
		{
			gpu_data.push_back(ChunkPrivate::m_front_verticies[index]);
			gpu_data.push_back(ChunkPrivate::m_front_verticies[index + 1]);
			gpu_data.push_back(ChunkPrivate::m_front_verticies[index + 2]);
			gpu_data.push_back(ChunkPrivate::m_front_verticies[index + 3]);
			gpu_data.push_back(ChunkPrivate::m_front_verticies[index + 4]);
			index += 5;
		}
	}
	{
		int index = 0;
		for (int i = 0; i < 6; i++)
		{
			gpu_data.push_back(ChunkPrivate::m_left_verticies[index]);
			gpu_data.push_back(ChunkPrivate::m_left_verticies[index + 1]);
			gpu_data.push_back(ChunkPrivate::m_left_verticies[index + 2]);
			gpu_data.push_back(ChunkPrivate::m_left_verticies[index + 3]);
			gpu_data.push_back(ChunkPrivate::m_left_verticies[index + 4]);
			index += 5;
		}
	}
	{
		int index = 0;
		for (int i = 0; i < 6; i++)
		{
			gpu_data.push_back(ChunkPrivate::m_right_verticies[index]);
			gpu_data.push_back(ChunkPrivate::m_right_verticies[index + 1]);
			gpu_data.push_back(ChunkPrivate::m_right_verticies[index + 2]);
			gpu_data.push_back(ChunkPrivate::m_right_verticies[index + 3]);
			gpu_data.push_back(ChunkPrivate::m_right_verticies[index + 4]);
			index += 5;
		}
	}
	{
		int index = 0;
		for (int i = 0; i < 6; i++)
		{
			gpu_data.push_back(ChunkPrivate::m_bottom_verticies[index]);
			gpu_data.push_back(ChunkPrivate::m_bottom_verticies[index + 1]);
			gpu_data.push_back(ChunkPrivate::m_bottom_verticies[index + 2]);
			gpu_data.push_back(ChunkPrivate::m_bottom_verticies[index + 3]);
			gpu_data.push_back(ChunkPrivate::m_bottom_verticies[index + 4]);
			index += 5;
		}
	}
	{
		int index = 0;
		for (int i = 0; i < 6; i++)
		{
			gpu_data.push_back(ChunkPrivate::m_top_verticies[index]);
			gpu_data.push_back(ChunkPrivate::m_top_verticies[index + 1]);
			gpu_data.push_back(ChunkPrivate::m_top_verticies[index + 2]);
			gpu_data.push_back(ChunkPrivate::m_top_verticies[index + 3]);
			gpu_data.push_back(ChunkPrivate::m_top_verticies[index + 4]);
			index += 5;
		}
	}


	glGenVertexArrays(1, &outline_b.vao);
	glBindVertexArray(outline_b.vao);

	glGenBuffers(1, &outline_b.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, outline_b.vbo);
	glBufferData(GL_ARRAY_BUFFER, gpu_data.size() * sizeof(int), gpu_data.data(), GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 5 * BLOCK_SIZE_BYTES, (void*)0);
	glEnableVertexAttribArray(0);

	// Texture coord
	glVertexAttribPointer(1, 2, GL_UNSIGNED_INT, GL_FALSE, 5 * BLOCK_SIZE_BYTES, (void*)(3 * BLOCK_SIZE_BYTES));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
//
//void outline_render(shader_program* s, glm::mat4 p, glm::mat4 v, glm::mat4 m)
//{
//	shader_use(s);
//	shader_set_mat4(s, "projection", p);
//	shader_set_mat4(s, "view", v);
//	shader_set_mat4(s, "model", m);
//
//	glBindVertexArray(outline_b.vao);
//
//	glDrawArrays(GL_TRIANGLES, 0, 36);
//	glBindVertexArray(0);
//}


#pragma region RAY_SHIT
inline glm::vec3 intbound(glm::vec3 s, glm::vec3 ds)
{
	glm::vec3 res;
	for (size_t i = 0; i < 3; i++)
	{
		res[i] =
			(ds[i] > 0 ?
				(glm::ceil(s[i]) - s[i])
				: (s[i] - glm::floor(s[i])))
			/ glm::abs(ds[i]);
	}
	return res;
}

struct ray_hit_result
{
	glm::ivec3 block_pos;
	glm::ivec3 direction;
	chunk* chunk_hit;
	glm::ivec2 chunk_world_pos;
};

struct Ray {
	glm::vec3 origin, direction;

	Ray() = default;
	Ray(const glm::vec3& o, const glm::vec3& d)
		: origin(o), direction(d)
	{
	}

	ray_hit_result intersect_block(float max_distance, bool inside = false)
	{
		ray_hit_result result;
		glm::ivec3 p, step;
		glm::vec3 t_max, t_delta;
		float radius;

		p = glm::floor(this->origin);
		step = glm::sign(this->direction);
		t_max = intbound(this->origin, this->direction);
		t_delta = glm::vec3(step) / this->direction;
		radius = max_distance / glm::l2Norm(this->direction);
		glm::ivec3 d = glm::ivec3(0);

		while (true)
		{
			//Check if we hit something, return block if we did
			for (auto& it : chunks)
			{
				if (p.x < it.first.x + CHUNK_SIZE_WIDTH && p.x > it.first.x &&
					p.z < it.first.y + CHUNK_SIZE_WIDTH && p.z > it.first.y)
				{
					int x = p.x - it.first.x;
					int y = p.y;
					int z = p.z - it.first.y;

					auto& a = it.second.blocks[to_1d_array(x, y, z)];
					if (a.type != block_type::AIR)
					{
						result.block_pos = glm::ivec3(x, y, z);
						result.chunk_hit = &it.second;
						result.chunk_world_pos = it.first;
						result.direction = d;
						return result;
					}
				}
			}

			if (t_max.x < t_max.y)
			{
				if (t_max.x < t_max.z)
				{
					if (t_max.x > radius)
					{
						break;
					}

					p.x += step.x;
					t_max.x += t_delta.x;
					d = glm::ivec3(-step.x, 0, 0);
				}
				else
				{
					if (t_max.z > radius)
					{
						break;
					}

					p.z += step.z;
					t_max.z += t_delta.z;
					d = glm::ivec3(0, 0, -step.z);
				}
			}
			else
			{
				if (t_max.y < t_max.z)
				{
					if (t_max.y > radius)
					{
						break;
					}

					p.y += step.y;
					t_max.y += t_delta.y;
					d = glm::ivec3(0, -step.y, 0);
				}
				else
				{
					if (t_max.z > radius)
					{
						break;
					}

					p.z += step.z;
					t_max.z += t_delta.z;
					d = glm::ivec3(0, 0, -step.z);
				}
			}
		}

		result.chunk_hit = nullptr;
		return result;
	}
};

void handle_block_hit(ray_hit_result ray_hit, bool remove)
{
	if (ray_hit.chunk_hit == nullptr)
		return;

	block_type type = block_type::STONE;
	if (remove)
		type = block_type::AIR;

	chunk* hit_chunk;
	glm::vec3 b_pos;

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
	if (another_chunk)
	{
		auto a = steady_clock::now();
		b_pos = glm::vec3((ray_hit.block_pos.x - CHUNK_SIZE_WIDTH - 1), ray_hit.block_pos.y, ray_hit.block_pos.z);

		if (!remove)
			b_pos += ray_hit.direction;

		chunks[ray_hit.chunk_hit->right_neighbor->world_pos].blocks[to_1d_array(b_pos)].type = type;

		auto aa = steady_clock::now();
		auto aaa = duration_cast<milliseconds>(aa - a).count();
		std::cout << "modify(another chunk): " << aaa << "ms\n";

		chunk* c = &chunks[ray_hit.chunk_hit->right_neighbor->world_pos];
		ChunkPrivate::generate_mesh(c, ray_hit.chunk_hit->right_neighbor->world_pos);
		glDeleteBuffers(1, &c->vbo_handle);
		glDeleteVertexArrays(1, &c->vao_handle);
		ChunkPrivate::init_buffers(c);
	}
	else
	{
		auto a = steady_clock::now();
		b_pos = ray_hit.block_pos;

		if (!remove)
			b_pos += ray_hit.direction;

		hit_chunk->blocks[to_1d_array(b_pos)].type = type;
		auto kys = hit_chunk->blocks[to_1d_array(b_pos)];

		hit_chunk->blocks_in_use = 0;

		auto c_gen_meshs = steady_clock::now();
		ChunkPrivate::generate_mesh_timed(hit_chunk, hit_chunk->world_pos);
		auto c_gen_meshst = steady_clock::now();

		auto c_del_buffers = steady_clock::now();
		glDeleteBuffers(1, &hit_chunk->vbo_handle);
		glDeleteVertexArrays(1, &hit_chunk->vao_handle);
		auto c_del_bufferst = steady_clock::now();

		auto c_inits = steady_clock::now();
		ChunkPrivate::init_buffers(hit_chunk);
		auto c_initst = steady_clock::now();

		std::cout << "genmesh: " << duration_cast<milliseconds>(c_gen_meshst - c_gen_meshs).count() << "ms\n";
		std::cout << "del buffers: " << duration_cast<milliseconds>(c_del_bufferst - c_del_buffers).count() << "ms\n";
		std::cout << "init_buffers: " << duration_cast<milliseconds>(c_initst - c_inits).count() << "ms\n";
	}
}

#pragma endregion

int lightingXD = 0;
int lightingshaderID = 0;
int main()
{
	auto window = init_and_create_window();

	set_opengl_constants();

	// build and comple our shader zprogram
	//shader_program lighting_shader;
	//shader_load(lighting_shader, )

	Shader lightingShader("resources\\opaque_world.shadervs", "resources\\opaque_world.shaderfs");
	lightingshaderID = lightingShader.ID;
	Shader outlineShader("resources\\outline.shadervs", "resources\\outline.shaderfs");
	Shader crosshairShader("resources\\crosshair.shadervs", "resources\\crosshair.shaderfs");

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

	const auto [atlas, texture2] = load_textures();

	create_outline();
	crosshair_t crosshair = crosshair_create();

	memory_arena_init(&block_arena, (sizeof(block) * BLOCKS_IN_CHUNK) * CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE);
	memory_arena_init(&noise_arena, sizeof(float) * ((CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT * CHUNK_SIZE_WIDTH) * CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE));
	memory_arena_init(&chunk_arena, sizeof(block_size_t) * ((CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT * CHUNK_SIZE_WIDTH) * CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE));

	init_chunks();

	memory_arena_dealloc(&noise_arena);

	int nbFrames = 0;
	double lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;

		// measure frames
		nbFrames++;
		if (currentTime - lastTime >= 1.0)
		{
			double msPerFrame = 1000.0 / double(nbFrames);
			double ms = 1000 / msPerFrame;
			nbFrames = 0;
			lastTime += 1.0;
		}

		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);

		Ray r(camera.Position, camera.Front);
		ray_hit_result result = r.intersect_block(10);
		if (result.chunk_hit != nullptr)
		{
			outlineShader.use();
			outlineShader.setMat4("projection", projection);
			outlineShader.setMat4("view", view);

			outlineShader.setMat4("model", glm::translate(glm::mat4(1.0f),
				glm::vec3(result.chunk_world_pos.x + result.block_pos.x,
					result.block_pos.y,
					result.chunk_world_pos.y + result.block_pos.z)));

			glBindVertexArray(outline_b.vao);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}

		lightingShader.use();

		int location = glGetUniformLocation(lightingshaderID, "lightingXD");
		glUniform1i(location, lightingXD);

		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);
		lightingShader.setMat4("model", model);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, atlas);

		for (auto& iter : chunks)
		{
			lightingShader.setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(iter.first.x, 0, iter.first.y)));
			ChunkPrivate::draw(iter.second);
		}

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		crosshair_draw(&crosshair, SCR_WIDTH, SCR_HEIGHT);

		//outline_render(outlineShader, projection, view, model);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
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

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "opengltest", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
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

std::tuple<GLuint, GLuint> load_textures()
{
	unsigned int texture2;
	unsigned char* dirt = load_png("resources\\new\\dirt.png");
	unsigned char* dirt_grass_side = load_png("resources\\new\\dirt_grass_side.png");
	unsigned char* dirt_grass_top = load_png("resources\\new\\dirt_grass_top.png");
	unsigned char* stone = load_png("resources\\new\\stone.png");
	unsigned char* sand = load_png("resources\\new\\sand.png");
	unsigned char* water = load_png("resources\\new\\water.png");
	unsigned char* leaves = load_png("resources\\new\\leaves.png");
	unsigned char* oak_log = load_png("resources\\new\\oak_log.png");
	unsigned char* oak_log_top = load_png("resources\\new\\oak_log_top.png");

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

	return std::make_tuple(atlas, texture2);
}
#pragma endregion

#pragma region CALLBACKS
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		lightingXD = 1;
	}
	else if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		lightingXD = 0;
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// find the smallest possible t such that s + t * ds is an integer

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		// viewport, 0 top of screen
		double x;
		double y;
		glfwGetCursorPos(window, &x, &y);

		auto position = camera.Position;
		auto rotation = camera.Front;

		Ray r(camera.Position, camera.Front);
		ray_hit_result result = r.intersect_block(100);

		handle_block_hit(result, true);
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		// viewport, 0 top of screen
		double x;
		double y;
		glfwGetCursorPos(window, &x, &y);

		auto position = camera.Position;
		auto rotation = camera.Front;

		Ray r(camera.Position, camera.Front);
		ray_hit_result result = r.intersect_block(100);

		handle_block_hit(result, false);
	}
}
#pragma endregion

