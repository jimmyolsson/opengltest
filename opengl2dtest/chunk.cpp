#include "chunk.h"

#include "glad/glad.h"

#include "util/common.h"

#include <glm/gtx/hash.hpp>
#include <chrono>
#include <vector>

const int ATTRIBUTES_PER_VERTEX = 1;

inline static int to_1d_array(const glm::ivec3 pos)
{
	return (pos.z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (pos.y * CHUNK_SIZE_WIDTH) + pos.x;
}
inline static int to_1d_array(const short x, const short y, const short z)
{
	return (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x;
}

block* chunk_get_block(Chunk* c, glm::ivec3 block_pos)
{
	return chunk_get_block(c, block_pos.x, block_pos.y, block_pos.z);
}

block* chunk_get_block(Chunk* chunk, short x, short y, short z)
{
	Chunk* c = chunk;
	if (x == 32)
	{
		c = chunk->right_neighbor;
		x = 0;
	}
	if (x == -1)
	{
		c = chunk->left_neighbor;
		x = 31;
	}
	if (z == -1)
	{
		c = chunk->back_neighbor;
		z = 31;
	}
	if (z == 32)
	{
		c = chunk->front_neighbor;
		z = 0;
	}

	if (c == nullptr)
		return nullptr;
	return &c->blocks[to_1d_array(x, y, z)];
}

void chunk_generate_buffers_transparent(Chunk* c)
{
	glGenVertexArrays(1, &c->vao_handle_transparent);
	glBindVertexArray(c->vao_handle_transparent);

	glGenBuffers(1, &c->vbo_handle_transparent);
	glBindBuffer(GL_ARRAY_BUFFER, c->vbo_handle_transparent);
	glBufferData(GL_ARRAY_BUFFER, c->gpu_data_arr_transparent.size() * sizeof(block_size_t), c->gpu_data_arr_transparent.data(), GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, ATTRIBUTES_PER_VERTEX * BLOCK_SIZE_BYTES, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	c->gpu_data_arr_transparent.clear();
}

void update_buffers(Chunk* chunk)
{
	glBindVertexArray(chunk->vao_handle);
	glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_handle);

	glBufferSubData(GL_ARRAY_BUFFER, 0, chunk->gpu_data_arr.size() * sizeof(block_size_t), &chunk->gpu_data_arr[0]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void chunk_generate_buffers(Chunk* chunk)
{
	glGenVertexArrays(1, &chunk->vao_handle);
	glBindVertexArray(chunk->vao_handle);

	glGenBuffers(1, &chunk->vbo_handle);
	glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_handle);
	glBufferData(GL_ARRAY_BUFFER, chunk->gpu_data_arr.size() * sizeof(int), chunk->gpu_data_arr.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, ATTRIBUTES_PER_VERTEX * BLOCK_SIZE_BYTES, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	chunk->gpu_data_arr.clear();

	chunk_generate_buffers_transparent(chunk);
}

void chunk_generate_buffers_new(Chunk* chunk)
{
	glGenVertexArrays(1, &chunk->vao_handle);
	glBindVertexArray(chunk->vao_handle);

	glGenBuffers(1, &chunk->vbo_handle);
	glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_handle);

	glBufferData(GL_ARRAY_BUFFER, chunk->gpu_data_arr.size() * sizeof(GPUData), chunk->gpu_data_arr.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GPUData), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(GPUData), (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	chunk->gpu_data_arr.clear();

	chunk_generate_buffers_transparent(chunk);
}


// The pos argument assumes that you are 'looking' at the back-face
bool find_inc(Chunk* chunk, glm::vec3 block_pos, glm::vec3 pos)
{
	glm::vec3 a = block_pos + pos;
	block* b = chunk_get_block(chunk, a);
	if (b != nullptr)
		if (!(b->type == BlockType::AIR || b->type == BlockType::WATER))
			return true;
	return false;
}

void calc_ao(Chunk* chunk, int i, BlockFaceDirection direction, glm::vec3 block_pos, char* occlusion)
{
	enum TriangleIndicies
	{
		TOP_LEFT = 0,
		TOP_RIGHT = 10,
		BOTTOM_LEFT = 15,
		BOTTOM_RIGHT = 20,
	};
	struct AOCoordinates
	{
		glm::vec3 first;
		glm::vec3 second;
		glm::vec3 third;
	};

	static const robin_hood::unordered_flat_map<glm::ivec2, AOCoordinates> ao_map =
	{
		{ { TriangleIndicies::TOP_LEFT,     BlockFaceDirection::BACK },		{ {0, 1, -1}, {1, 0, -1}, {1, 1, -1} } },
		{ { TriangleIndicies::TOP_LEFT,     BlockFaceDirection::FRONT },	{ {-1, 1, 1}, {-1, 0, 1}, {0, 1, 1} } },
		{ { TriangleIndicies::TOP_LEFT,     BlockFaceDirection::TOP },		{ {-1, 1, 1}, {-1, 1, 0}, {0, 1, 1} } },
		{ { TriangleIndicies::TOP_LEFT,		BlockFaceDirection::BOTTOM },	{ {0, -1, 1}, {1, -1, 1}, {1, -1, 0} } },
		{ { TriangleIndicies::TOP_LEFT,     BlockFaceDirection::RIGHT },	{ {1, 1, 0}, {1, 0, 1}, {1, 1, 1} } },
		{ { TriangleIndicies::TOP_LEFT,     BlockFaceDirection::LEFT },		{ {-1, 1, -1}, {-1, 0, -1 }, { -1, 1, 0} } },
		{ { TriangleIndicies::TOP_RIGHT,    BlockFaceDirection::BACK },		{ {-1, 1, -1}, {-1, 0, -1}, {0, 1, -1} } },
		{ { TriangleIndicies::TOP_RIGHT,    BlockFaceDirection::FRONT },	{ {0, 1, 1}, {1, 0, 1}, {1, 1, 1} } },
		{ { TriangleIndicies::TOP_RIGHT,    BlockFaceDirection::TOP },		{ {-1, 1, 0}, {-1, 1, -1}, {0, 1, -1} } },
		{ { TriangleIndicies::TOP_RIGHT,    BlockFaceDirection::BOTTOM },	{ {0, -1, -1}, {1, -1, -1}, {1, -1, 0} } },
		{ { TriangleIndicies::TOP_RIGHT,    BlockFaceDirection::RIGHT },	{ {1, 1, -1}, {1, 0, -1}, {1, 1, 0} } },
		{ { TriangleIndicies::TOP_RIGHT,    BlockFaceDirection::LEFT },		{ {-1, 1, 1}, {-1, 0, 1 }, { -1, 1, 0} } },
		{ { TriangleIndicies::BOTTOM_LEFT,  BlockFaceDirection::BACK },		{ {0, -1, -1}, {1, 0, -1}, {1, -1, -1} } },
		{ { TriangleIndicies::BOTTOM_LEFT,  BlockFaceDirection::FRONT },	{ {-1, -1, 1}, {-1, 0, 1}, {0, -1, 1} } },
		{ { TriangleIndicies::BOTTOM_LEFT,  BlockFaceDirection::TOP },		{ {0, 1, 1}, {1, 1, 1}, {1, 1, 0} } },
		{ { TriangleIndicies::BOTTOM_LEFT,  BlockFaceDirection::BOTTOM },	{ {-1, -1, 1}, {-1, -1, 0}, {0, -1, 1} } },
		{ { TriangleIndicies::BOTTOM_LEFT,  BlockFaceDirection::RIGHT },	{ {1, -1, 0}, {1, 0, 1}, {1, -1, 1} } },
		{ { TriangleIndicies::BOTTOM_LEFT,  BlockFaceDirection::LEFT },		{ {-1, -1, -1}, {-1, 0, -1 }, { -1, -1, 0} } },
		{ { TriangleIndicies::BOTTOM_RIGHT, BlockFaceDirection::BACK },		{ {-1, -1, -1}, {-1, 0, -1}, {0, -1, -1} } },
		{ { TriangleIndicies::BOTTOM_RIGHT, BlockFaceDirection::FRONT },	{ {0, -1, 1}, {1, 0, 1}, {1, -1, 1} } },
		{ { TriangleIndicies::BOTTOM_RIGHT, BlockFaceDirection::TOP },		{ {0, 1, -1}, {1, 1, -1}, {1, 1, 0} } },
		{ { TriangleIndicies::BOTTOM_RIGHT, BlockFaceDirection::BOTTOM },	{ {-1, -1, 0}, {-1, -1, -1}, {0, -1, -1} } },
		{ { TriangleIndicies::BOTTOM_RIGHT, BlockFaceDirection::RIGHT },	{ {1, -1, -1}, {1, 0, -1}, {1, -1, 0} } },
		{ { TriangleIndicies::BOTTOM_RIGHT, BlockFaceDirection::LEFT },		{ {-1, -1, 0}, {-1, 0, 1 }, { -1, -1, 1} } }
	};

	// Duplicate indicies, we dont use indexes
	if (i == 25)
		i = 10;
	else if (i == 5)
		i = 15;

	const AOCoordinates* coord = &ao_map.at(glm::ivec2(i, direction));
	*occlusion +=
		find_inc(chunk, block_pos, coord->first) +
		find_inc(chunk, block_pos, coord->second) +
		find_inc(chunk, block_pos, coord->third);
}

//void add_face_and_texture(Chunk* chunk, const block_size_t* data, BlockFaceDirection direction, int x, int y, int z)
//{
//	const glm::vec3 block_pos = glm::vec3(x, y, z);
//
//	static const int vert_count = 30;
//	int i = 0;
//	while (i != vert_count)
//	{
//		char occlusion = 0;
//		calc_ao(chunk, i, direction, block_pos, &occlusion);
//
//		unsigned char texture = block_get_texture(direction, chunk->blocks[to_1d_array(x, y, z)].type);
//
//		// yyyyyyyy zzzzzzxx xxxxuvoo bbbbbbbl
//		unsigned int result = 0;
//		result = (data[i + 1] + y) << 24;
//		result |= (data[i] + x) << 18;
//		result |= (data[i + 2] + z) << 12;
//		result |= (data[i + 3]) << 11;
//		result |= (data[i + 4]) << 10;
//		result |= occlusion << 8;
//		result |= texture << 1;
//		auto lighting = direction == BlockFaceDirection::LEFT || direction == BlockFaceDirection::RIGHT ? 1 : 0;
//		result |= lighting;
//
//		//chunk->gpu_data_arr.push_back(result);
//		i += 5;
//	}
//}

void add_stair_mesh(const block_size_t* data)
{
}

void add_face_and_texture_new(Chunk* chunk, const block_size_t* data, BlockFaceDirection direction, int x, int y, int z, BlockType type)
{
	const glm::vec3 block_pos = glm::vec3(x, y, z);

	if (type == BlockType::STONE)
	{
		static const int vert_count = 30;
		int i = 0;
		while (i != vert_count)
		{
			char occlusion = 0;
			calc_ao(chunk, i, direction, block_pos, &occlusion);

			unsigned char texture = block_get_texture(direction, type);

			GPUData result;
			result.x = (data[i]/2) + x;
			result.y = data[i + 1] + y;
			result.z = data[i + 2] + z;

			unsigned int u = (int)data[i + 3];
			unsigned int v = (int)data[i + 4];

			result.info = 0;
			result.info |= u << 11;
			result.info |= v << 10;
			result.info |= occlusion << 8;
			result.info |= texture << 1;
			auto lighting = direction == BlockFaceDirection::LEFT || direction == BlockFaceDirection::RIGHT ? 1 : 0;
			result.info |= lighting;

			chunk->gpu_data_arr.push_back(result);
			i += 5;
		}
	}
	else
	{
		static const int vert_count = 30;
		int i = 0;
		while (i != vert_count)
		{
			char occlusion = 0;
			calc_ao(chunk, i, direction, block_pos, &occlusion);

			unsigned char texture = block_get_texture(direction, type);

			GPUData result;
			result.x = data[i] + x;
			result.y = data[i + 1] + y;
			result.z = data[i + 2] + z;

			unsigned int u = (int)data[i + 3];
			unsigned int v = (int)data[i + 4];

			result.info = 0;
			result.info |= u << 11;
			result.info |= v << 10;
			result.info |= occlusion << 8;
			result.info |= texture << 1;
			auto lighting = direction == BlockFaceDirection::LEFT || direction == BlockFaceDirection::RIGHT ? 1 : 0;
			result.info |= lighting;

			chunk->gpu_data_arr.push_back(result);
			i += 5;
		}
	}
}

void generate_face(Chunk* current_chunk, const Chunk* neighbor, const block_size_t data[30], BlockFaceDirection direction, int x, int y, int z, int other_chunk_index, int current_chunk_index, bool on_edge, BlockType type)
{
	if (on_edge)
	{
		//If i have a neighbor that is transparent
		if (neighbor != nullptr && block_is_transparent(neighbor->blocks[other_chunk_index].type))
		{
			add_face_and_texture_new(current_chunk, data, direction, x, y, z, type);
		}
	}
	// Is the block next to me occupied?
	else if (block_is_transparent(current_chunk->blocks[current_chunk_index].type))
	{
		add_face_and_texture_new(current_chunk, data, direction, x, y, z, type);
	}
}

void chunk_set_block(Chunk* c, glm::ivec3 block_pos, BlockType new_type)
{
	if (c == nullptr)
		return;

	c->blocks[to_1d_array(block_pos)].type = new_type;
	c->dirty = true;

	if (block_pos.x == CHUNK_SIZE_WIDTH - 1 && c->right_neighbor != nullptr)
		c->right_neighbor->dirty = true;
	else if (block_pos.x == 0 && c->left_neighbor != nullptr)
		c->left_neighbor->dirty = true;
	else if (block_pos.z == CHUNK_SIZE_WIDTH - 1 && c->front_neighbor != nullptr)
		c->front_neighbor->dirty = true;
	else if (block_pos.z == 0 && c->back_neighbor != nullptr)
		c->back_neighbor->dirty = true;
}

void check_chunk_neighbors(Chunk* chunk)
{
	if (chunk->back_neighbor == nullptr && chunk->chunks->contains(glm::ivec2((int)chunk->world_pos.x, (int)chunk->world_pos.y - CHUNK_SIZE_WIDTH)))
	{
		chunk->back_neighbor = &chunk->chunks->at(glm::ivec2((int)chunk->world_pos.x, (int)chunk->world_pos.y - CHUNK_SIZE_WIDTH));
	}
	if (chunk->front_neighbor == nullptr && chunk->chunks->contains(glm::ivec2((int)chunk->world_pos.x, (int)chunk->world_pos.y + CHUNK_SIZE_WIDTH)))
	{
		chunk->front_neighbor = &chunk->chunks->at(glm::ivec2((int)chunk->world_pos.x, (int)chunk->world_pos.y + CHUNK_SIZE_WIDTH));
	}
	if (chunk->left_neighbor == nullptr && chunk->chunks->contains(glm::ivec2((int)chunk->world_pos.x - CHUNK_SIZE_WIDTH, (int)chunk->world_pos.y)))
	{
		chunk->left_neighbor = &chunk->chunks->at(glm::ivec2((int)chunk->world_pos.x - CHUNK_SIZE_WIDTH, (int)chunk->world_pos.y));
	}
	if (chunk->right_neighbor == nullptr && chunk->chunks->contains(glm::ivec2((int)chunk->world_pos.x + CHUNK_SIZE_WIDTH, (int)chunk->world_pos.y)))
	{
		chunk->right_neighbor = &chunk->chunks->at(glm::ivec2((int)chunk->world_pos.x + CHUNK_SIZE_WIDTH, (int)chunk->world_pos.y));
	}
}

void add_face_and_texture_t(Chunk* chunk, const block_size_t* data, BlockFaceDirection direction, int x, int y, int z)
{
	//const int vert_count = 30;
	//int i = 0;
	//while (i != vert_count)
	//{
	//	unsigned char texture = block_get_texture(direction, chunk->blocks[to_1d_array(x, y, z)].type);
	//	unsigned int result = 0;
	//	result = (data[i + 1] + y) << 24;
	//	result |= (data[i] + x) << 18;
	//	result |= (data[i + 2] + z) << 12;
	//	result |= (data[i + 3]) << 11;
	//	result |= (data[i + 4]) << 10;
	//	result |= texture << 1;

	//	chunk->gpu_data_arr_transparent.push_back(result);
	//	i += 5;
	//}
	const glm::vec3 block_pos = glm::vec3(x, y, z);

	static const int vert_count = 30;
	int i = 0;
	while (i != vert_count)
	{
		char occlusion = 0;
		calc_ao(chunk, i, direction, block_pos, &occlusion);

		unsigned char texture = block_get_texture(direction, chunk->blocks[to_1d_array(x, y, z)].type);

		GPUData result;
		result.x = data[i] + x;
		result.y = data[i + 1] + y;
		result.z = data[i + 2] + z;

		unsigned int u = (int)data[i + 3];
		unsigned int v = (int)data[i + 4];

		result.info = 0;
		result.info |= u << 11;
		result.info |= v << 10;
		result.info |= occlusion << 8;
		result.info |= texture << 1;
		auto lighting = direction == BlockFaceDirection::LEFT || direction == BlockFaceDirection::RIGHT ? 1 : 0;
		result.info |= lighting;

		chunk->gpu_data_arr.push_back(result);
		i += 5;
	}

}

void generate_face_t(Chunk* current_chunk, const Chunk* neighbor, const block_size_t data[30], BlockFaceDirection direction, int x, int y, int z, int other_chunk_index, int current_chunk_index, bool on_edge)
{
	if (on_edge)
	{
		//If i have a neighbor that is transparent
		if (neighbor != nullptr && !block_is_translucent(neighbor->blocks[other_chunk_index].type))
		{
			add_face_and_texture_t(current_chunk, data, direction, x, y, z);
		}
	}
	// Is the block next to me occupied?
	else if (!block_is_translucent(current_chunk->blocks[current_chunk_index].type))
	{
		add_face_and_texture_t(current_chunk, data, direction, x, y, z);
	}
}

void gen_mesh_translucent(Chunk* chunk)
{
	check_chunk_neighbors(chunk);

	for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
	{
		for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
		{
			for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
			{
				int current = to_1d_array(x, y, z);
				if (chunk->blocks[current].type != BlockType::WATER)
					continue;

				generate_face_t(chunk, chunk->back_neighbor, m_back_verticies, BlockFaceDirection::BACK, x, y, z, to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1), to_1d_array(x, y, z - 1), z == 0);
				generate_face_t(chunk, chunk->front_neighbor, m_front_verticies, BlockFaceDirection::FRONT, x, y, z, to_1d_array(x, y, 0), to_1d_array(x, y, z + 1), (z + 1) >= CHUNK_SIZE_WIDTH);

				generate_face_t(chunk, chunk->left_neighbor, m_left_verticies, BlockFaceDirection::LEFT, x, y, z, to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z), to_1d_array(x - 1, y, z), x == 0);
				generate_face_t(chunk, chunk->right_neighbor, m_right_verticies, BlockFaceDirection::RIGHT, x, y, z, to_1d_array(0, y, z), to_1d_array(x + 1, y, z), ((x + 1) >= CHUNK_SIZE_WIDTH));

				// no chunk neighbors on the Y-axis
				if (y != 0 && (y == 0 || chunk->blocks[to_1d_array(x, y - 1, z)].type != BlockType::WATER))
				{
					add_face_and_texture_t(chunk, m_bottom_verticies, BlockFaceDirection::BOTTOM, x, y, z);
				}

				if (y + 1 > CHUNK_SIZE_HEIGHT || chunk->blocks[to_1d_array(x, y + 1, z)].type == BlockType::AIR)
				{
					add_face_and_texture_t(chunk, m_top_verticies, BlockFaceDirection::TOP, x, y, z);
				}
			}
		}
	}
}

void gen_mesh_opaque(Chunk* chunk)
{
	chunk->gpu_data_arr.clear();
	using namespace std::chrono;
	check_chunk_neighbors(chunk);
	for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
	{
		for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
		{
			for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
			{
				int current = to_1d_array(x, y, z);
				auto type = chunk->blocks[current].type;

				if (type == BlockType::AIR || type == BlockType::WATER)
					continue;

				//if (a == BlockType::GLASS_PANE)
				//{
				//	generate_face(chunk, chunk->back_neighbor, m_pane_back, BlockFaceDirection::BACK, x, y, z, to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1), to_1d_array(x, y, z - 1), z == 0, type);
				//	generate_face(chunk, chunk->front_neighbor, m_pane_front, BlockFaceDirection::FRONT, x, y, z, to_1d_array(x, y, 0), to_1d_array(x, y, z + 1), (z + 1) >= CHUNK_SIZE_WIDTH - 1, type);

				//	generate_face(chunk, chunk->left_neighbor, m_pane_left, BlockFaceDirection::LEFT, x, y, z, to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z), to_1d_array(x - 1, y, z), x == 0, type);
				//	generate_face(chunk, chunk->right_neighbor, m_pane_right, BlockFaceDirection::RIGHT, x, y, z, to_1d_array(0, y, z), to_1d_array(x + 1, y, z), ((x + 1) >= CHUNK_SIZE_WIDTH - 1), type);

				//	// no chunk neighbors on the Y-axis
				//	if (y != 0 && (y == 0 || block_is_transparent(chunk->blocks[to_1d_array(x, y - 1, z)].type)))
				//	{
				//		add_face_and_texture_new(chunk, m_pane_bottom, BlockFaceDirection::BOTTOM, x, y, z, type);
				//	}

				//	if (y + 1 > CHUNK_SIZE_HEIGHT || block_is_transparent(chunk->blocks[to_1d_array(x, y + 1, z)].type))
				//	{
				//		add_face_and_texture_new(chunk, m_pane_top, BlockFaceDirection::TOP, x, y, z, type);
				//	}
				//}
				//else
				//{
					generate_face(chunk, chunk->back_neighbor, m_back_verticies, BlockFaceDirection::BACK, x, y, z, to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1), to_1d_array(x, y, z - 1), z == 0, type);
					generate_face(chunk, chunk->front_neighbor, m_front_verticies, BlockFaceDirection::FRONT, x, y, z, to_1d_array(x, y, 0), to_1d_array(x, y, z + 1), (z + 1) >= CHUNK_SIZE_WIDTH - 1, type);

					generate_face(chunk, chunk->left_neighbor, m_left_verticies, BlockFaceDirection::LEFT, x, y, z, to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z), to_1d_array(x - 1, y, z), x == 0, type);
					generate_face(chunk, chunk->right_neighbor, m_right_verticies, BlockFaceDirection::RIGHT, x, y, z, to_1d_array(0, y, z), to_1d_array(x + 1, y, z), ((x + 1) >= CHUNK_SIZE_WIDTH - 1), type);

					// no chunk neighbors on the Y-axis
					if (y != 0 && (y == 0 || block_is_transparent(chunk->blocks[to_1d_array(x, y - 1, z)].type)))
					{
						add_face_and_texture_new(chunk, m_bottom_verticies, BlockFaceDirection::BOTTOM, x, y, z, type);
					}

					if (y + 1 > CHUNK_SIZE_HEIGHT || block_is_transparent(chunk->blocks[to_1d_array(x, y + 1, z)].type))
					{
						add_face_and_texture_new(chunk, m_top_verticies, BlockFaceDirection::TOP, x, y, z, type);
					}
				//}
			}
		}
	}
}

void chunk_generate_mesh(Chunk* chunk)
{
	gen_mesh_opaque(chunk);
	gen_mesh_translucent(chunk);
	chunk->verts_in_use = chunk->gpu_data_arr.size();
	chunk->verts_in_use_transparent = chunk->gpu_data_arr_transparent.size();
}

void calculate_lighting(Chunk* c, const glm::vec2& chunk_pos)
{
	for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
	{
		for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
		{
			for (int y = CHUNK_SIZE_HEIGHT - 1; y != 0; --y)
			{
				block* b = &c->blocks[to_1d_array(x, y, z)];
				if (b->type != BlockType::AIR)
				{
					b->sky = true;
					break;
				}
			}
		}
	}
}

void chunk_update(chunk_map_t* chunks)
{
	for (decltype(auto) it : *chunks)
	{
		if (it.second.dirty)
		{
			chunk_generate_mesh(&it.second);
			//update_buffers(&it.second);
			glDeleteBuffers(1, &it.second.vbo_handle);
			glDeleteVertexArrays(1, &it.second.vao_handle);
			chunk_generate_buffers_new(&it.second);
			it.second.dirty = false;
			it.second.gpu_data_arr.clear();
		}
	}
}

void chunk_render(Chunk* chunk, Renderer* renderer, glm::mat4 view, glm::vec3 position)
{
	if (chunk->dirty)
		return;

	renderer_render_custom(renderer,
		view,
		TEXTURE_ATLAS_CHUNK,
		SHADER_CHUNK,
		chunk->vao_handle,
		chunk->verts_in_use,
		position,
		glm::vec3(1));
}

void chunk_render_transparent(Chunk* chunk, Renderer* renderer, glm::mat4 view, glm::vec3 position)
{
	if (chunk->dirty)
		return;

	renderer_render_custom(renderer,
		view,
		TEXTURE_ATLAS_CHUNK,
		SHADER_CHUNK,
		chunk->vao_handle_transparent,
		chunk->verts_in_use_transparent,
		position,
		glm::vec3(1));
}
