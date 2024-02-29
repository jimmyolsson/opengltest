#include "chunk.h"

//#include "util/common_graphics.h"

#include "util/common.h"

#include <glm/gtx/hash.hpp>
#include <chrono>
#include <vector>
#include <numeric>
#include "blocks/blocks.h"

const int ATTRIBUTES_PER_VERTEX = 1;

inline static int to_1d_array(const glm::ivec3& pos)
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
	if (x == CHUNK_SIZE_WIDTH)
	{
		c = chunk->right_neighbor;
		x = 0;
	}
	if (x == -1)
	{
		c = chunk->left_neighbor;
		x = CHUNK_SIZE_WIDTH - 1;
	}
	if (z == -1)
	{
		c = chunk->back_neighbor;
		z = CHUNK_SIZE_WIDTH - 1;
	}
	if (z == CHUNK_SIZE_WIDTH)
	{
		c = chunk->front_neighbor;
		z = 0;
	}

	if (c == nullptr)
		return nullptr;
	return &c->blocks[to_1d_array(x, y, z)];
}

void generate_buffers(unsigned int* vao_handle, unsigned int* vbo_handle, std::vector<GPUData>* gpu_data)
{
	glGenVertexArrays(1, &*vao_handle);
	glBindVertexArray(*vao_handle);

	glGenBuffers(1, &*vbo_handle);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo_handle);

	glBufferData(GL_ARRAY_BUFFER, gpu_data->size() * sizeof(GPUData), gpu_data->data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GPUData), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(GPUData), (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void chunk_generate_buffers(Chunk* self)
{
	generate_buffers(&self->vao_handle_opaque, &self->vbo_handle_opaque, &self->gpu_data_opaque);
	generate_buffers(&self->vao_handle_transparent, &self->vbo_handle_transparent, &self->gpu_data_transparent);
	generate_buffers(&self->vao_handle_veg, &self->vbo_handle_veg, &self->gpu_data_veg);
}

// The pos argument assumes that you are looking at the back-face
bool find_inc(Chunk* chunk, glm::vec3 block_pos, glm::vec3 pos)
{
	glm::vec3 a = block_pos + pos;

	block* b = chunk_get_block(chunk, a);
	if (b != nullptr)
		if (!block_infos[b->type].is_transparent)
			return true;
	return false;
}

// Calculates the ambient occlusion for the given vertex
// TODO: Rewrite, this is completely idiotic and written entirely on a whim
char calc_ao(Chunk* chunk, int i, BlockFaceDirection direction, glm::ivec3 block_pos)
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
		glm::ivec3 first;
		glm::ivec3 second;
		glm::ivec3 third;
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

	return find_inc(chunk, block_pos, coord->first) +
		find_inc(chunk, block_pos, coord->second) +
		find_inc(chunk, block_pos, coord->third);
}

void add_face_and_texture_new(Chunk* chunk, std::vector<GPUData>* gpu_data, const block_size_t* data, BlockFaceDirection direction, int x, int y, int z, BlockType type)
{
	const glm::vec3 block_pos = glm::vec3(x, y, z);

	static const int vert_count = 30;
	int i = 0;
	while (i != vert_count)
	{
		unsigned char texture = (char)block_get_texture(direction, type);

		GPUData result;
		result.x = data[i] + x;
		result.y = data[i + 1] + y;
		result.z = data[i + 2] + z;

		unsigned int u = (int)data[i + 3];
		unsigned int v = (int)data[i + 4];

		char ao = 0;
		if (block_infos[type].ao_enabled)
			ao = calc_ao(chunk, i, direction, block_pos);

		if (type == BlockType::WATER)
			int a = 2;

		result.info = 0;
		result.info |= u << 11;
		result.info |= v << 10;
		result.info |= ao << 8;
		result.info |= texture << 1;
		auto lighting = direction == BlockFaceDirection::LEFT || direction == BlockFaceDirection::RIGHT ? 1 : 0;
		result.info |= lighting;

		gpu_data->push_back(result);
		i += 5;
	}
}
//
//void add_face_and_texture_f(Chunk* chunk, const block_size_t* data, int x, int y, int z, BlockType type)
//{
//	const glm::vec3 block_pos = glm::vec3(x, y, z);
//
//	static const int vert_count = 30;
//	int i = 0;
//	while (i != vert_count)
//	{
//		char occlusion = 0;
//
//		unsigned char texture = (char)BlockTextureIndex::GRASS;
//
//		GPUData result;
//		result.x = data[i] + x;
//		result.y = data[i + 1] + y;
//		result.z = data[i + 2] + z;
//
//		unsigned int u = (int)data[i + 3];
//		unsigned int v = (int)data[i + 4];
//
//		result.info = 0;
//		result.info |= u << 11;
//		result.info |= v << 10;
//		result.info |= occlusion << 8;
//		result.info |= texture << 1;
//		auto lighting = 0;// = direction == BlockFaceDirection::LEFT || direction == BlockFaceDirection::RIGHT ? 1 : 0;
//		result.info |= lighting;
//
//		chunk->gpu_data_arr_transparent.push_back(result);
//		i += 5;
//	}
//}
//
//void add_face_and_texture_t(Chunk* chunk, const block_size_t* data, BlockFaceDirection direction, int x, int y, int z, BlockType type)
//{
//	const glm::vec3 block_pos = glm::vec3(x, y, z);
//
//	static const int vert_count = 30;
//	int i = 0;
//	while (i != vert_count)
//	{
//		char occlusion = 0;
//
//		unsigned char texture = block_get_texture(direction, type);
//
//		GPUData result;
//		result.x = data[i] + x;
//		result.y = data[i + 1] + y;
//		result.z = data[i + 2] + z;
//
//		unsigned int u = (int)data[i + 3];
//		unsigned int v = (int)data[i + 4];
//
//		result.info = 0;
//		result.info |= u << 11;
//		result.info |= v << 10;
//		result.info |= occlusion << 8;
//		result.info |= texture << 1;
//		auto lighting = direction == BlockFaceDirection::LEFT || direction == BlockFaceDirection::RIGHT ? 1 : 0;
//		result.info |= lighting;
//
//		chunk->gpu_data_arr_transparent.push_back(result);
//		i += 5;
//	}
//}

void generate_face(Chunk* current_chunk, std::vector<GPUData>* gpu_data, const Chunk* neighbor, const block_size_t data[30], BlockFaceDirection direction, int x, int y, int z, int other_chunk_index, int current_chunk_index, bool on_edge, BlockType type)
{
	if (on_edge)
	{
		//If i have a neighbor that is transparent
		if (neighbor != nullptr && block_infos[neighbor->blocks[other_chunk_index].type].is_transparent)
		{
			add_face_and_texture_new(current_chunk, gpu_data, data, direction, x, y, z, type);
		}

	}
	// Is the block next to me occupied?
	else if (block_infos[current_chunk->blocks[current_chunk_index].type].is_transparent)
	{
		add_face_and_texture_new(current_chunk, gpu_data, data, direction, x, y, z, type);
	}
}

void generate_face_t(Chunk* current_chunk, std::vector<GPUData>* gpu_data, const Chunk* neighbor, const block_size_t data[30], BlockFaceDirection direction, int x, int y, int z, int other_chunk_index, int current_chunk_index, bool on_edge, BlockType type)
{
	if (on_edge)
	{
		//If i have a neighbor that is transparent
		if (neighbor != nullptr && neighbor->blocks[other_chunk_index].type == BlockType::AIR)
		{
			add_face_and_texture_new(current_chunk, gpu_data, data, direction, x, y, z, type);
		}
	}
	// Is the block next to me occupied?
	else if (current_chunk->blocks[current_chunk_index].type == BlockType::AIR)
	{
		add_face_and_texture_new(current_chunk, gpu_data, data, direction, x, y, z, type);
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

// Also generates partially transparent(translucent) blocks
void gen_mesh_transparent(Chunk* chunk)
{
	for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
	{
		for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
		{
			for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
			{
				int current = to_1d_array(x, y, z);
				BlockType type = chunk->blocks[current].type;

				switch (type)
				{
					case BlockType::LEAVES:
					{
						// This creates flickering because we have duplicate triangles, cull backface?
						add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_back_verticies, BlockFaceDirection::BACK, x, y, z, type);
						add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_front_verticies, BlockFaceDirection::FRONT, x, y, z, type);
						add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_left_verticies, BlockFaceDirection::LEFT, x, y, z, type);
						add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_right_verticies, BlockFaceDirection::RIGHT, x, y, z, type);
						add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_bottom_verticies, BlockFaceDirection::BOTTOM, x, y, z, type);
						add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_top_verticies, BlockFaceDirection::TOP, x, y, z, type);
						break;
					}
					case BlockType::GRASS:
					{
						// Grass is a bit weird since it has no 'direction' so it's just a placeholder here.
						add_face_and_texture_new(chunk, &chunk->gpu_data_veg, grass_1, BlockFaceDirection::BACK, x, y, z, type);
						add_face_and_texture_new(chunk, &chunk->gpu_data_veg, grass_2, BlockFaceDirection::BACK, x, y, z, type);
						break;
					}
					// Default behaviour
					case BlockType::WATER:
					case BlockType::GLASS_PANE:
					{
						generate_face_t(chunk, &chunk->gpu_data_transparent, chunk->back_neighbor, m_back_verticies, BlockFaceDirection::BACK, x, y, z, to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1), to_1d_array(x, y, z - 1), z == 0, type);
						generate_face_t(chunk, &chunk->gpu_data_transparent, chunk->front_neighbor, m_front_verticies, BlockFaceDirection::FRONT, x, y, z, to_1d_array(x, y, 0), to_1d_array(x, y, z + 1), (z + 1) >= CHUNK_SIZE_WIDTH, type);

						generate_face_t(chunk, &chunk->gpu_data_transparent, chunk->left_neighbor, m_left_verticies, BlockFaceDirection::LEFT, x, y, z, to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z), to_1d_array(x - 1, y, z), x == 0, type);
						generate_face_t(chunk, &chunk->gpu_data_transparent, chunk->right_neighbor, m_right_verticies, BlockFaceDirection::RIGHT, x, y, z, to_1d_array(0, y, z), to_1d_array(x + 1, y, z), ((x + 1) >= CHUNK_SIZE_WIDTH), type);

						// no chunk neighbors on the Y-axis
						if (y != 0 && (y == 0 || chunk->blocks[to_1d_array(x, y - 1, z)].type == BlockType::AIR))
						{
							add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_bottom_verticies, BlockFaceDirection::BOTTOM, x, y, z, type);
						}

						if (y + 1 > CHUNK_SIZE_HEIGHT || chunk->blocks[to_1d_array(x, y + 1, z)].type == BlockType::AIR)
						{
							add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_top_verticies, BlockFaceDirection::TOP, x, y, z, type);
						}
						break;
					}
					case BlockType::GLASS:
					{
						generate_face(chunk, &chunk->gpu_data_transparent, chunk->back_neighbor, m_back_verticies, BlockFaceDirection::BACK, x, y, z, to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1), to_1d_array(x, y, z - 1), z == 0, type);
						generate_face(chunk, &chunk->gpu_data_transparent, chunk->front_neighbor, m_front_verticies, BlockFaceDirection::FRONT, x, y, z, to_1d_array(x, y, 0), to_1d_array(x, y, z + 1), z >= CHUNK_SIZE_WIDTH - 1, type);

						generate_face(chunk, &chunk->gpu_data_transparent, chunk->left_neighbor, m_left_verticies, BlockFaceDirection::LEFT, x, y, z, to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z), to_1d_array(x - 1, y, z), x == 0, type);
						generate_face(chunk, &chunk->gpu_data_transparent, chunk->right_neighbor, m_right_verticies, BlockFaceDirection::RIGHT, x, y, z, to_1d_array(0, y, z), to_1d_array(x + 1, y, z), x >= CHUNK_SIZE_WIDTH - 1, type);

						// no chunk neighbors on the Y-axis
						if (y != 0 && (y == 0 || block_infos[chunk->blocks[to_1d_array(x, y - 1, z)].type].is_transparent))
						{
							add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_bottom_verticies, BlockFaceDirection::BOTTOM, x, y, z, type);
						}

						if (y + 1 > CHUNK_SIZE_HEIGHT || block_infos[chunk->blocks[to_1d_array(x, y + 1, z)].type].is_transparent)
						{
							add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_top_verticies, BlockFaceDirection::TOP, x, y, z, type);
						}

						break;
					}
				}
			}
		}
	}
}

void gen_mesh_opaque(Chunk* chunk)
{
	chunk->gpu_data_opaque.clear();

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

				if (block_infos[type].is_transparent)
					continue;

				generate_face(chunk, &chunk->gpu_data_opaque, chunk->back_neighbor, m_back_verticies, BlockFaceDirection::BACK, x, y, z, to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1), to_1d_array(x, y, z - 1), z == 0, type);
				generate_face(chunk, &chunk->gpu_data_opaque, chunk->front_neighbor, m_front_verticies, BlockFaceDirection::FRONT, x, y, z, to_1d_array(x, y, 0), to_1d_array(x, y, z + 1), z >= CHUNK_SIZE_WIDTH - 1, type);

				generate_face(chunk, &chunk->gpu_data_opaque, chunk->left_neighbor, m_left_verticies, BlockFaceDirection::LEFT, x, y, z, to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z), to_1d_array(x - 1, y, z), x == 0, type);
				generate_face(chunk, &chunk->gpu_data_opaque, chunk->right_neighbor, m_right_verticies, BlockFaceDirection::RIGHT, x, y, z, to_1d_array(0, y, z), to_1d_array(x + 1, y, z), x >= CHUNK_SIZE_WIDTH - 1, type);

				// no chunk neighbors on the Y-axis
				if (y != 0 && (y == 0 || block_infos[chunk->blocks[to_1d_array(x, y - 1, z)].type].is_transparent))
				{
					add_face_and_texture_new(chunk, &chunk->gpu_data_opaque, m_bottom_verticies, BlockFaceDirection::BOTTOM, x, y, z, type);
				}

				if (y + 1 > CHUNK_SIZE_HEIGHT || block_infos[chunk->blocks[to_1d_array(x, y + 1, z)].type].is_transparent)
				{
					add_face_and_texture_new(chunk, &chunk->gpu_data_opaque, m_top_verticies, BlockFaceDirection::TOP, x, y, z, type);
				}
			}
		}
	}
}

void chunk_generate_mesh(Chunk* chunk)
{
	chunk->gpu_data_opaque.clear();
	chunk->gpu_data_transparent.clear();
	chunk->gpu_data_veg.clear();

	// TODO: These should be the same method
	gen_mesh_opaque(chunk);

	gen_mesh_transparent(chunk);

	chunk->verts_in_use = chunk->gpu_data_opaque.size();
	chunk->verts_in_use_transparent = chunk->gpu_data_transparent.size();
	chunk->verts_in_use_veg = chunk->gpu_data_veg.size();
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
					break;
				}
			}
		}
	}
}

void update_buffers(GLuint vao_handle, const std::vector<GPUData>& gpu_data_arr)
{
	glBindBuffer(GL_ARRAY_BUFFER, vao_handle);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GPUData) * gpu_data_arr.size(), gpu_data_arr.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void _clear_chunk_gpu_buffers(Chunk* self)
{
	glDeleteBuffers(1, &self->vbo_handle_opaque);
	glDeleteVertexArrays(1, &self->vao_handle_opaque);

	glDeleteBuffers(1, &self->vbo_handle_transparent);
	glDeleteVertexArrays(1, &self->vao_handle_transparent);

	glDeleteBuffers(1, &self->vbo_handle_veg);
	glDeleteVertexArrays(1, &self->vao_handle_veg);
}

glm::vec3 calculate_centroid(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3)
{
	return (v1 + v2 + v3) / 3.0f;
}

void _sort_vertex_buffer(std::vector<GPUData>* gpu_data, glm::vec3 camera_position)
{
	std::vector<float> distances;
	for (int i = 0; i < gpu_data->size(); i += 3)
	{
		const glm::vec3 vfirst = glm::vec3(
			(*gpu_data)[i].x,
			(*gpu_data)[i].y,
			(*gpu_data)[i].z);
		const glm::vec3 vsecond = glm::vec3(
			(*gpu_data)[i + 1].x,
			(*gpu_data)[i + 1].y,
			(*gpu_data)[i + 1].z);
		const glm::vec3 vthird = glm::vec3(
			(*gpu_data)[i + 2].x,
			(*gpu_data)[i + 2].y,
			(*gpu_data)[i + 2].z);

		float distance = glm::distance(calculate_centroid(vfirst, vsecond, vthird), camera_position);
		distances.push_back(distance);
	}

	std::vector<std::size_t> indexes(distances.size());
	std::iota(indexes.begin(), indexes.end(), 0);

	std::sort(indexes.begin(), indexes.end(), [&](std::size_t a, std::size_t b)
	{
		return distances[a] > distances[b];
	});

	std::vector<GPUData> sortedVertices(gpu_data->size());

	for (std::size_t i = 0; i < indexes.size(); ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			sortedVertices[i * 3 + j] = (*gpu_data)[indexes[i] * 3 + j];
		}
	}

	*gpu_data = std::move(sortedVertices);
}

void _chunk_sort_triangles(chunk_map_t* chunks, glm::vec3 camera_pos)
{
	for (auto& c : *chunks)
	{
		_sort_vertex_buffer(&c.second.gpu_data_transparent, camera_pos);
		_sort_vertex_buffer(&c.second.gpu_data_veg, camera_pos);
	}
}

void chunk_update(chunk_map_t* chunks, glm::vec3 camera_pos)
{
	_chunk_sort_triangles(chunks, camera_pos);
	for (auto& pair : *chunks)
	{
		Chunk& chunk = pair.second;
		if (chunk.dirty)
		{
			_clear_chunk_gpu_buffers(&chunk);
			chunk_generate_mesh(&chunk);
			chunk_generate_buffers(&pair.second);
			chunk.dirty = false;
		}
		else
		{
			update_buffers(chunk.vao_handle_transparent, chunk.gpu_data_transparent);
		}
	}
}

void chunk_render_opaque(Chunk* chunk, Renderer* renderer, glm::mat4 view, glm::vec3 position)
{
	if (chunk->dirty)
		return;

	renderer_render_custom(renderer,
		view,
		TEXTURE_ATLAS_CHUNK,
		SHADER_CHUNK,
		chunk->vao_handle_opaque,
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

void chunk_render_veg(Chunk* chunk, Renderer* renderer, glm::mat4 view, glm::vec3 position)
{
	if (chunk->dirty)
		return;

	renderer_render_custom(renderer,
		view,
		TEXTURE_ATLAS_CHUNK,
		SHADER_CHUNK,
		chunk->vao_handle_veg,
		chunk->verts_in_use_veg,
		position,
		glm::vec3(1));
}
