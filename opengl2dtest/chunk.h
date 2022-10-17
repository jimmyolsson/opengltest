#pragma once
#include <glm/glm.hpp>
#include "robin_hood.h"

#include <vector>
#include "blocks/block.h"
#include "graphics/shader.h"

#include "common.h"

enum block_type;
enum class block_face_direction;

const int CHUNK_SIZE_WIDTH = 32;
const int CHUNK_SIZE_HEIGHT = 255;
//const int CHUNK_SIZE_WIDTH = 1;
//const int CHUNK_SIZE_HEIGHT = 1;
#if _DEBUG
const int CHUNK_DRAW_DISTANCE = 2;
#else
const int CHUNK_DRAW_DISTANCE = 10;
#endif
const int TOTAL_CHUNKS = CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE;
const int BLOCKS_IN_CHUNK = CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT * CHUNK_SIZE_WIDTH;

// this needs to match gl type

struct chunk
{
	block* blocks;
	robin_hood::unordered_flat_map<glm::ivec2, chunk>* chunks;
	glm::ivec2 world_pos;

	block_size_t* gpu_data_arr = nullptr;
	int blocks_in_use = 0;

	block_size_t* gpu_data_arr_transparent = nullptr;
	int blocks_in_use_transparent = 0;

	chunk* front_neighbor = nullptr;
	chunk* back_neighbor = nullptr;
	chunk* right_neighbor = nullptr;
	chunk* left_neighbor = nullptr;

	unsigned int vao_handle = -1;
	unsigned int vbo_handle = -1;

	unsigned int vao_handle_transparent = -1;
	unsigned int vbo_handle_transparent = -1;

	shader_program shader;

	bool initialized = false;
	bool dirty = false;
};

// Exposed just so that we can multithread init
void chunk_generate_mesh(chunk* chunk);
void chunk_generate_buffers(chunk* chunk);
void chunk_set_block(chunk* c, glm::ivec3 block_pos, block_type new_type);
block* chunk_get_block(chunk* c, glm::ivec3 block_pos);
block* chunk_get_block(chunk* c, short x, short y, short z);
void chunk_update(chunk_map_t* chunks);
void chunk_render(const chunk& chunk);
