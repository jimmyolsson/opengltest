#pragma once
#include <glm/glm.hpp>
#include "robin_hood.h"

#include <vector>

struct block;
enum class block_type;
enum class block_face_direction;

//const int CHUNK_SIZE_WIDTH = 2;
//const int CHUNK_SIZE_HEIGHT = 2;
//const int CHUNK_DRAW_DISTANCE = 2;
const int CHUNK_SIZE_WIDTH = 16;
const int CHUNK_SIZE_HEIGHT = 50;
const int CHUNK_DRAW_DISTANCE = 12;

static int to_1d_array(int x, int y, int z)
{
	return (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x;
}

struct chunk
{
	block* blocks;

	const robin_hood::unordered_node_map<glm::ivec2, chunk>* chunks;

	std::vector<float> gpu_data;

	unsigned int vao_handle;
	unsigned int vbo_handle;

	//inline std::size_t operator()(const chunk& key) const
	//{
	//	return std::hash<int>()(key.chunkCoords.x) ^
	//		std::hash<int>()(key.chunkCoords.y);
	//}
	//inline bool operator==(const chunk& other) const
	//{
	//	return chunkCoords == other.chunkCoords;
	//}

	//inline bool operator!=(const chunk& other) const
	//{
	//	return !(*this == other);
	//}

	//inline bool operator==(const glm::ivec2& other) const
	//{
	//	return chunkCoords == other;
	//}

	//inline bool operator!=(const glm::ivec2& other) const
	//{
	//	return !(*this == other);
	//}
};

namespace ChunkPrivate
{
	void init_buffers(chunk& chunk);
	void generate_mesh(chunk& chunk, const glm::vec2& chunk_pos);
	void draw(chunk& chunk);

	const float m_back_verticies[30] = {
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
	};
	const float m_front_verticies[30] = {
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
	};
	const float m_left_verticies[30] = {
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	};
	const float m_right_verticies[30] = {
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
	};
	const float m_bottom_verticies[30] = {
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
	};
	const float m_top_verticies[30] = {
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
	};
}
