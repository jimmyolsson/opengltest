#pragma once
#include <glm/glm.hpp>
#include "robin_hood.h"

#include <vector>
#include "blocks/block.h"
#include "graphics/renderer.h"
#include "common.h"

enum BlockType;
enum class block_face_direction;

static const int CHUNK_SIZE_WIDTH = 32;
static const int CHUNK_SIZE_HEIGHT = 255;
#if _DEBUG
const int CHUNK_DRAW_DISTANCE = 8;
#else
//const int CHUNK_DRAW_DISTANCE = 30;
static const int CHUNK_DRAW_DISTANCE = 10;
#endif
static const int TOTAL_CHUNKS = CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE;
static const int BLOCKS_IN_CHUNK = CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT * CHUNK_SIZE_WIDTH;

struct Chunk
{
	char* block_types;

	char* block_neighbors;
	block* blocks;
	robin_hood::unordered_flat_map<glm::ivec2, Chunk>* chunks;
	glm::ivec2 world_pos;

	block_size_t* gpu_data_arr = nullptr;
	int blocks_in_use = 0;

	block_size_t* gpu_data_arr_transparent = nullptr;
	int blocks_in_use_transparent = 0;

	Chunk* front_neighbor = nullptr;
	Chunk* back_neighbor = nullptr;
	Chunk* right_neighbor = nullptr;
	Chunk* left_neighbor = nullptr;

	unsigned int vao_handle = -1;
	unsigned int vbo_handle = -1;

	unsigned int vao_handle_transparent = -1;
	unsigned int vbo_handle_transparent = -1;

	ShaderProgram shader;

	bool initialized = false;
	bool dirty = false;
};

void chunk_set_block(Chunk* c, glm::ivec3 block_pos, BlockType new_type);

// If the coordinates are outside of the bounds of the chunk, it will return the neighboring chunk
block* chunk_get_block(Chunk* c, glm::ivec3 block_pos);
block* chunk_get_block(Chunk* c, short x, short y, short z);

// Exposed just so that we can multithread init
void chunk_generate_mesh(Chunk* chunk);
void chunk_generate_buffers(Chunk* chunk);

void chunk_generate_mesh_transparent(Chunk* chunk);
void chunk_generate_buffers_transparent(Chunk* chunk);

void chunk_update(chunk_map_t* chunks);
void chunk_render(Chunk* chunk, Renderer* renderer, glm::mat4 view, glm::vec3 position, int enabled);
void chunk_render_transparent(Chunk* chunk, Renderer* renderer, glm::mat4 view, glm::vec3 position, int enabled);
