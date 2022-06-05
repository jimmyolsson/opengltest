#pragma once
#include <glm/glm.hpp>
#include "robin_hood.h"

#include <vector>

struct block;
enum class block_type : int;
enum class block_face_direction;

const int CHUNK_SIZE_WIDTH = 128;
const int CHUNK_SIZE_HEIGHT = 128 + 128;
#if _DEBUG
const int CHUNK_DRAW_DISTANCE = 2;
#else
const int CHUNK_DRAW_DISTANCE = 10;
#endif
const int TOTAL_ELEMENTS_IN_QUAD = 48;
const int TOTAL_CHUNKS = CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE;

// this needs to match gl type
typedef int block_size_t;
const int BLOCK_SIZE_BYTES = sizeof(block_size_t);

struct chunk
{
	block* blocks;

	const robin_hood::unordered_flat_map<glm::ivec2, chunk>* chunks;

	block_size_t* gpu_data_arr = nullptr;
	int gpu_data_length = 0;
	int gpu_data_used = 0;
	int gpu_data_last_used = 0;

	const chunk* front_neighbor = nullptr;
	const chunk* back_neighbor = nullptr;
	const chunk* right_neighbor = nullptr;
	const chunk* left_neighbor = nullptr;

	unsigned int vao_handle = -1;
	unsigned int vbo_handle = -1;

	bool initialized = false;
};

namespace ChunkPrivate
{
	void update_buffers(chunk& chunk);
	void init_buffers(chunk& chunk);
	void generate_mesh(chunk& chunk, const glm::vec2& chunk_pos);
	void draw(const chunk& chunk);

	// x y z nx ny nz u v
	const static block_size_t m_back_verticies[TOTAL_ELEMENTS_IN_QUAD] = {
		0, 0, 0,  0,  0, -1,  0,  0,
		 1, 0, 0,  0,  0, -1,  1,  0,
		 1,  1, 0,  0,  0, -1,  1,  1,
		 1,  1, 0,  0,  0, -1,  1,  1,
		0,  1, 0,  0,  0, -1,  0,  1,
		0, 0, 0,  0,  0, -1,  0,  0,
	};
	const static block_size_t m_front_verticies[TOTAL_ELEMENTS_IN_QUAD] = {
		0, 0,  1,  0,  0,  1,  0,  0,
		 1, 0,  1,  0,  0,  1,  1,  0,
		 1,  1,  1,  0,  0,  1,  1,  1,
		 1,  1,  1,  0,  0,  1,  1,  1,
		0,  1,  1,  0,  0,  1,  0,  1,
		0, 0,  1,  0,  0,  1,  0,  0,
	};
	const static block_size_t m_left_verticies[TOTAL_ELEMENTS_IN_QUAD] = {
		0,  1,  1, -1,  0,  0,  1,  1,
		0,  1, 0, -1,  0,  0,  0,  1,
		0, 0, 0, -1,  0,  0,  0,  0,
		0, 0, 0, -1,  0,  0,  0,  0,
		0, 0,  1, -1,  0,  0,  1,  0,
		0,  1,  1, -1,  0,  0,  1,  1,
	};
	const static block_size_t m_right_verticies[TOTAL_ELEMENTS_IN_QUAD] = {
		 1,  1,  1,  1,  0,  0,  1,  1,
		 1,  1, 0,  1,  0,  0,  0,  1,
		 1, 0, 0,  1,  0,  0,  0,  0,
		 1, 0, 0,  1,  0,  0,  0,  0,
		 1, 0,  1,  1,  0,  0,  1,  0,
		 1,  1,  1,  1,  0,  0,  1,  1,
	};
	const static block_size_t m_bottom_verticies[TOTAL_ELEMENTS_IN_QUAD] = {
		0, 0, 0,  0, -1,  0,  0,  1,
		 1, 0, 0,  0, -1,  0,  1,  1,
		 1, 0,  1,  0, -1,  0,  1,  0,
		 1, 0,  1,  0, -1,  0,  1,  0,
		0, 0,  1,  0, -1,  0,  0,  0,
		0, 0, 0,  0, -1,  0,  0,  1,
	};
	const static block_size_t m_top_verticies[TOTAL_ELEMENTS_IN_QUAD] = {
		0,  1, 0,  0,  1,  0,  0,  1,
		 1,  1, 0,  0,  1,  0,  1,  1,
		 1,  1,  1,  0,  1,  0,  1,  0,
		 1,  1,  1,  0,  1,  0,  1,  0,
		0,  1,  1,  0,  1,  0,  0,  0,
		0,  1, 0,  0,  1,  0,  0,  1
	};
}
