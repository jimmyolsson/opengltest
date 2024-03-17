#include "chunk.h"

//#include "util/common_graphics.h"

#include "util/common.h"

#include <glm/gtx/hash.hpp>
#include <chrono>
#include <vector>
#include <numeric>
#include <queue>
#include "blocks/blocks.h"
#include "ray.h"
#include "selectable_profiler.cpp"

const int ATTRIBUTES_PER_VERTEX = 1;
static bool light_reload = true;

glm::ivec3 to_3d_position(int index)
{
	int z = index / (CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT);
	index -= (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT);
	int y = index / CHUNK_SIZE_WIDTH;
	int x = index % CHUNK_SIZE_WIDTH;

	return glm::ivec3(x, y, z);
}

static int to_1d_array(const glm::ivec3& pos)
{
	return (pos.z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (pos.y * CHUNK_SIZE_WIDTH) + pos.x;
}

static int to_1d_array(const short x, const short y, const short z)
{
	auto a = (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x;
	return a;
}

Block* chunk_get_block_inside(Chunk* chunk, short x, short y, short z)
{
	if (x < 0 || y < 0 || z < 0 ||
		x > CHUNK_SIZE_WIDTH - 1 || y > CHUNK_SIZE_HEIGHT - 1 || z > CHUNK_SIZE_WIDTH - 1)
	{
		return nullptr;
	}
	else
	{
		return &chunk->blocks[to_1d_array(x, y, z)];
	}
}

Block* chunk_get_block_inside(Chunk* c, glm::ivec3 block_pos)
{
	return chunk_get_block_inside(c, block_pos.x, block_pos.y, block_pos.z);
}

GetBlockResult chunk_get_block(Chunk* c, glm::ivec3 block_pos)
{
	return chunk_get_block(c, block_pos.x, block_pos.y, block_pos.z);
}

GetBlockResult chunk_get_blockf(Chunk* chunk, short x, short y, short z)
{
	GetBlockResult result = { nullptr, -1, nullptr };
	Chunk* c = chunk;

	// Adjust for neighboring chunks
	if (x < 0)
	{
		c = chunk->left_neighbor;
		x += CHUNK_SIZE_WIDTH;
	}
	else if (x >= CHUNK_SIZE_WIDTH)
	{
		c = chunk->right_neighbor;
		x -= CHUNK_SIZE_WIDTH;
	}

	if (z < 0)
	{
		c = chunk->back_neighbor;
		z += CHUNK_SIZE_WIDTH;
	}
	else if (z >= CHUNK_SIZE_WIDTH)
	{
		c = chunk->front_neighbor;
		z -= CHUNK_SIZE_WIDTH;
	}

	// Check for valid chunk and y-coordinate
	if (c == nullptr || y < 0 || y >= CHUNK_SIZE_HEIGHT)
	{
		return result;
	}

	// Calculate index and return result
	int index = to_1d_array(glm::ivec3(x, y, z));
	result.b = &c->blocks[index];
	result.block_index = index;
	result.c = c;

	return result;
}

GetBlockResult chunk_get_block(Chunk* chunk, short x, short y, short z)
{
	GetBlockResult result = { 0 };
	Chunk* c = chunk;

	if (x == CHUNK_SIZE_WIDTH)
	{
		c = chunk->right_neighbor;
		x = 0;
	}
	else if (x == -1)
	{
		c = chunk->left_neighbor;
		x = CHUNK_SIZE_WIDTH - 1;
	}

	if (z == -1)
	{
		c = chunk->back_neighbor;
		z = CHUNK_SIZE_WIDTH - 1;
	}
	else if (z == CHUNK_SIZE_WIDTH)
	{
		c = chunk->front_neighbor;
		z = 0;
	}

	if (c == nullptr || y <= -1 || y >= CHUNK_SIZE_HEIGHT)
	{
		return { nullptr, {}, nullptr };
	}

	int index = to_1d_array(x, y, z);

	return
	{
		&c->blocks[index],
		index,
		c
	};
}
struct NeighborChunks
{
	glm::ivec2 coord;
	bool exist = false;
};

void get_surrounding_chunks(chunk_map_t* chunks, glm::ivec2 world_pos, NeighborChunks* positions)
{
	// Relative positions of the eight surrounding chunks
	const int offsets[8][2] = {
		{-1, -1}, { 0, -1}, { 1, -1},
		{-1,  0},           { 1,  0},
		{-1,  1}, { 0,  1}, { 1,  1}
	};

	for (int i = 0; i < 8; ++i)
	{
		positions[i].coord = world_pos + glm::ivec2(offsets[i][0] * CHUNK_SIZE_WIDTH, offsets[i][1] * CHUNK_SIZE_WIDTH);
		auto iter = chunks->find(positions[i].coord);
		positions[i].exist = iter != chunks->end();
	}
}

void chunk_set_full_dirty(Chunk* c)
{
	c->needs_light_recalc = true;
	c->needs_remesh = true;
}

void chunk_set_block(Chunk* c, int index, BlockType new_type)
{
	chunk_set_block(c, to_3d_position(index), new_type);
}

void chunk_set_block(Chunk* c, glm::ivec3 block_pos, BlockType new_type)
{
	Block* b = &c->blocks[to_1d_array(block_pos)];
	b->type = new_type;
	c->needs_remesh = true;

	if (new_type == BlockType::AIR)
	{
		b->light = 0;
	}

	// TODO: Better approach is to let the light algo define who actually needs a remesh
	if (block_infos[new_type].is_light_emitter)
	{
		NeighborChunks neighbors[8];
		get_surrounding_chunks(c->chunks, c->world_pos, neighbors);
		for (int i = 0; i < 8; i++)
		{
			if (neighbors[i].exist)
				c->chunks->at(neighbors[i].coord).needs_remesh = true;
		}
	}
	else
	{
		if (block_pos.x == CHUNK_SIZE_WIDTH - 1 && c->right_neighbor != nullptr)
			c->right_neighbor->needs_remesh = true;
		if (block_pos.x == 0 && c->left_neighbor != nullptr)
			c->left_neighbor->needs_remesh = true;
		if (block_pos.z == CHUNK_SIZE_WIDTH - 1 && c->front_neighbor != nullptr)
			c->front_neighbor->needs_remesh = true;
		if (block_pos.z == 0 && c->back_neighbor != nullptr)
			c->back_neighbor->needs_remesh = true;
	}
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

	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(GPUData), (void*)(sizeof(float) * 3 + sizeof(int)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void chunk_generate_buffers(Chunk* self)
{
	TimeFunction;
	generate_buffers(&self->vao_handle_opaque, &self->vbo_handle_opaque, &self->gpu_data_opaque);
	generate_buffers(&self->vao_handle_transparent, &self->vbo_handle_transparent, &self->gpu_data_transparent);
	generate_buffers(&self->vao_handle_veg, &self->vbo_handle_veg, &self->gpu_data_veg);
}

// The pos argument assumes that you are looking at the back-face
bool find_inc(Chunk* chunk, glm::vec3 block_pos, glm::vec3 pos)
{
	TimeFunction
		glm::vec3 a = block_pos + pos;

	Block* b = chunk_get_block(chunk, a).b;
	if (b != nullptr)
		if (!block_infos[b->type].is_transparent)
			return true;
	return false;
}

// Calculates the ambient occlusion for the given vertex
// TODO: Rewrite, this is completely idiotic and written entirely on a whim
char calc_ao(Chunk* chunk, int i, BlockFaceDirection direction, glm::ivec3 block_pos)
{
	TimeFunction;
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

void add_face_and_texture_new(Chunk* chunk, std::vector<GPUData>* gpu_data, const block_size_t* data, BlockFaceDirection direction, int x, int y, int z, const Block& block)
{
	TimeFunction;
	const glm::vec3 block_pos = glm::vec3(x, y, z);

	static const int vert_count = 30;
	int i = 0;
	while (i != vert_count)
	{
		unsigned char texture = (char)block_get_texture(direction, block.type);

		GPUData result = { 0 };
		result.x = data[i] + x;
		result.y = data[i + 1] + y;
		result.z = data[i + 2] + z;

		unsigned int u = (int)data[i + 3];
		unsigned int v = (int)data[i + 4];

		char ao = 0;
		if (block_infos[block.type].ao_enabled)
			ao = calc_ao(chunk, i, direction, block_pos);

		int direc = direction == BlockFaceDirection::LEFT || direction == BlockFaceDirection::RIGHT ? 1 : 0;
		char lighting_level = 5;
		if (chunk->world_pos.x == -32 && chunk->world_pos.y == 0)
			if (x == 31 && y == 41 && z == 0)
				int asd = 2;
		result.info |= u << 31;
		result.info |= v << 30;
		result.info |= ao << 28;
		result.info |= texture << 20;
		result.info |= direc << 19;

		if (block_infos[block.type].is_transparent)
			result.info |= block.light << 11;
		else
			result.info |= block.lighting_level[(int)direction] << 11;

		gpu_data->push_back(result);
		i += 5;
	}
}

void generate_face(Chunk* current_chunk, std::vector<GPUData>* gpu_data, const Chunk* neighbor, const block_size_t data[30], BlockFaceDirection direction, int x, int y, int z, int other_chunk_index, int current_chunk_index, bool on_edge, const Block& block)
{
	TimeFunction;
	if (on_edge)
	{
		//If i have a neighbor that is transparent
		if (neighbor != nullptr && block_infos[neighbor->blocks[other_chunk_index].type].is_transparent)
		{
			add_face_and_texture_new(current_chunk, gpu_data, data, direction, x, y, z, block);
		}

	}
	// Is the block next to me occupied?
	else if (block_infos[current_chunk->blocks[current_chunk_index].type].is_transparent)
	{
		add_face_and_texture_new(current_chunk, gpu_data, data, direction, x, y, z, block);
	}
}

void generate_face_t(Chunk* current_chunk, std::vector<GPUData>* gpu_data, const Chunk* neighbor, const block_size_t data[30], BlockFaceDirection direction, int x, int y, int z, int other_chunk_index, int current_chunk_index, bool on_edge, const Block& block)
{
	if (on_edge)
	{
		//If i have a neighbor that is transparent
		if (neighbor != nullptr && neighbor->blocks[other_chunk_index].type == BlockType::AIR)
		{
			add_face_and_texture_new(current_chunk, gpu_data, data, direction, x, y, z, block);
		}
	}
	// Is the block next to me occupied?
	else if (current_chunk->blocks[current_chunk_index].type == BlockType::AIR)
	{
		add_face_and_texture_new(current_chunk, gpu_data, data, direction, x, y, z, block);
	}
}

void set_chunk_neighbors(Chunk* chunk)
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
				const Block& current_block = chunk->blocks[current];

				switch (current_block.type)
				{
					case BlockType::LEAVES:
					{
						// This creates flickering because we have duplicate triangles, cull backface?
						add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_back_verticies, BlockFaceDirection::BACK, x, y, z, current_block);
						add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_front_verticies, BlockFaceDirection::FRONT, x, y, z, current_block);
						add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_left_verticies, BlockFaceDirection::LEFT, x, y, z, current_block);
						add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_right_verticies, BlockFaceDirection::RIGHT, x, y, z, current_block);
						add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_bottom_verticies, BlockFaceDirection::BOTTOM, x, y, z, current_block);
						add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_top_verticies, BlockFaceDirection::TOP, x, y, z, current_block);
						break;
					}
					case BlockType::GRASS:
					{
						// Grass is a bit weird since it has no 'direction' so it's just a placeholder here.
						add_face_and_texture_new(chunk, &chunk->gpu_data_veg, grass_1, BlockFaceDirection::BACK, x, y, z, current_block);
						add_face_and_texture_new(chunk, &chunk->gpu_data_veg, grass_2, BlockFaceDirection::BACK, x, y, z, current_block);
						break;
					}
					// Default behaviour
					case BlockType::WATER:
					case BlockType::GLASS_PANE:
					{
						generate_face_t(chunk, &chunk->gpu_data_transparent, chunk->back_neighbor, m_back_verticies, BlockFaceDirection::BACK, x, y, z, to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1), to_1d_array(x, y, z - 1), z == 0, current_block);
						generate_face_t(chunk, &chunk->gpu_data_transparent, chunk->front_neighbor, m_front_verticies, BlockFaceDirection::FRONT, x, y, z, to_1d_array(x, y, 0), to_1d_array(x, y, z + 1), (z + 1) >= CHUNK_SIZE_WIDTH, current_block);

						generate_face_t(chunk, &chunk->gpu_data_transparent, chunk->left_neighbor, m_left_verticies, BlockFaceDirection::LEFT, x, y, z, to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z), to_1d_array(x - 1, y, z), x == 0, current_block);
						generate_face_t(chunk, &chunk->gpu_data_transparent, chunk->right_neighbor, m_right_verticies, BlockFaceDirection::RIGHT, x, y, z, to_1d_array(0, y, z), to_1d_array(x + 1, y, z), ((x + 1) >= CHUNK_SIZE_WIDTH), current_block);

						// no chunk neighbors on the Y-axis
						if (y != 0 && (y == 0 || chunk->blocks[to_1d_array(x, y - 1, z)].type == BlockType::AIR))
						{
							add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_bottom_verticies, BlockFaceDirection::BOTTOM, x, y, z, current_block);
						}

						if (y + 1 > CHUNK_SIZE_HEIGHT || chunk->blocks[to_1d_array(x, y + 1, z)].type == BlockType::AIR)
						{
							add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_top_verticies, BlockFaceDirection::TOP, x, y, z, current_block);
						}
						break;
					}
					case BlockType::GLASS:
					{
						generate_face(chunk, &chunk->gpu_data_transparent, chunk->back_neighbor, m_back_verticies, BlockFaceDirection::BACK, x, y, z, to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1), to_1d_array(x, y, z - 1), z == 0, current_block);
						generate_face(chunk, &chunk->gpu_data_transparent, chunk->front_neighbor, m_front_verticies, BlockFaceDirection::FRONT, x, y, z, to_1d_array(x, y, 0), to_1d_array(x, y, z + 1), z >= CHUNK_SIZE_WIDTH - 1, current_block);

						generate_face(chunk, &chunk->gpu_data_transparent, chunk->left_neighbor, m_left_verticies, BlockFaceDirection::LEFT, x, y, z, to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z), to_1d_array(x - 1, y, z), x == 0, current_block);
						generate_face(chunk, &chunk->gpu_data_transparent, chunk->right_neighbor, m_right_verticies, BlockFaceDirection::RIGHT, x, y, z, to_1d_array(0, y, z), to_1d_array(x + 1, y, z), x >= CHUNK_SIZE_WIDTH - 1, current_block);

						// no chunk neighbors on the Y-axis
						if (y != 0 && (y == 0 || block_infos[chunk->blocks[to_1d_array(x, y - 1, z)].type].is_transparent))
						{
							add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_bottom_verticies, BlockFaceDirection::BOTTOM, x, y, z, current_block);
						}

						if (y + 1 > CHUNK_SIZE_HEIGHT || block_infos[chunk->blocks[to_1d_array(x, y + 1, z)].type].is_transparent)
						{
							add_face_and_texture_new(chunk, &chunk->gpu_data_transparent, m_top_verticies, BlockFaceDirection::TOP, x, y, z, current_block);
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
	TimeFunction;
	chunk->gpu_data_opaque.clear();

	using namespace std::chrono;
	for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
	{
		for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
		{
			for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
			{
				int current = to_1d_array(x, y, z);
				Block& current_block = chunk->blocks[current];

				if (block_infos[current_block.type].is_transparent)
					continue;

				if (x == 0 && y == 41 && z == 0)
					int i = 2;
				generate_face(chunk, &chunk->gpu_data_opaque, chunk->back_neighbor, m_back_verticies, BlockFaceDirection::BACK, x, y, z, to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1), to_1d_array(x, y, z - 1), z == 0, current_block);
				generate_face(chunk, &chunk->gpu_data_opaque, chunk->front_neighbor, m_front_verticies, BlockFaceDirection::FRONT, x, y, z, to_1d_array(x, y, 0), to_1d_array(x, y, z + 1), z >= CHUNK_SIZE_WIDTH - 1, current_block);

				generate_face(chunk, &chunk->gpu_data_opaque, chunk->left_neighbor, m_left_verticies, BlockFaceDirection::LEFT, x, y, z, to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z), to_1d_array(x - 1, y, z), x == 0, current_block);
				generate_face(chunk, &chunk->gpu_data_opaque, chunk->right_neighbor, m_right_verticies, BlockFaceDirection::RIGHT, x, y, z, to_1d_array(0, y, z), to_1d_array(x + 1, y, z), x >= CHUNK_SIZE_WIDTH - 1, current_block);

				// no chunk neighbors on the Y-axis
				if (y != 0 && (y == 0 || block_infos[chunk->blocks[to_1d_array(x, y - 1, z)].type].is_transparent))
				{
					add_face_and_texture_new(chunk, &chunk->gpu_data_opaque, m_bottom_verticies, BlockFaceDirection::BOTTOM, x, y, z, current_block);
				}

				if (y + 1 > CHUNK_SIZE_HEIGHT || block_infos[chunk->blocks[to_1d_array(x, y + 1, z)].type].is_transparent)
				{
					add_face_and_texture_new(chunk, &chunk->gpu_data_opaque, m_top_verticies, BlockFaceDirection::TOP, x, y, z, current_block);
				}
			}
		}
	}
}
struct LightNode
{
	int index;
	Chunk* chunk;

	LightNode(int idx, Chunk* ch) : index(idx), chunk(ch) {}
};

void lighting_compute_node(std::queue<LightNode>& light_nodes, LightNode& node, BlockFaceDirection dir)
{
	TimeFunction;
	int node_index = node.index;
	Chunk* node_chunk = node.chunk;
	glm::ivec3 node_pos = to_3d_position(node_index);
	int node_light_level = node_chunk->blocks[node_index].light;

	glm::ivec3 neighbor_pos = node_pos;

	if (dir == BlockFaceDirection::LEFT)
		neighbor_pos.x = node_pos.x + 1;
	else if (dir == BlockFaceDirection::RIGHT)
		neighbor_pos.x = node_pos.x - 1;
	else if (dir == BlockFaceDirection::BOTTOM)
		neighbor_pos.y = node_pos.y + 1;
	else if (dir == BlockFaceDirection::TOP)
		neighbor_pos.y = node_pos.y - 1;
	else if (dir == BlockFaceDirection::BACK)
		neighbor_pos.z = node_pos.z + 1;
	else if (dir == BlockFaceDirection::FRONT)
		neighbor_pos.z = node_pos.z - 1;

	// DEBUG
	if (node_chunk->world_pos.x == -32 && node_chunk->world_pos.y == 0)
	{
		int b = 2;
		if (node_pos.x == 31 && node_pos.y == 41 && node_pos.z == 1)
		{
			int a = 2;
		}
		if (neighbor_pos.x == 31 && neighbor_pos.y == 41 && neighbor_pos.z == 1)
		{
			int a = 2;
		}
	}

	int a = 2;
	auto neighbor = chunk_get_block(node_chunk, neighbor_pos);
	if (neighbor.c != nullptr && neighbor.b != nullptr)
	{
		// Make sure its not opaque
		if (block_is_transparent(neighbor.b->type) &&
			neighbor.b->light + 2 <= node_light_level)
		{
			neighbor.b->light = node_light_level - 1;
			//neighbor.c->needs_remesh = true;
			//neighbor.c->needs_light_reset = true;

			light_nodes.emplace(neighbor.block_index, neighbor.c);
		}
		// If it is opaque set the light level of the block-face that is facing the light
		else if (!block_is_transparent(neighbor.b->type))
		{
			neighbor.b->light = node_light_level - 1;;
			neighbor.b->lighting_level[(int)dir] = node_light_level - 1;
		}
	}
}
void lighting_run_bfs(Chunk* c, int index)
{
	std::queue<LightNode> light_nodes_queue = {};
	light_nodes_queue.emplace(index, c);

	while (light_nodes_queue.empty() == false)
	{
		LightNode node = light_nodes_queue.front();

		light_nodes_queue.pop();

		lighting_compute_node(light_nodes_queue, node, BlockFaceDirection::LEFT);
		lighting_compute_node(light_nodes_queue, node, BlockFaceDirection::RIGHT);
		lighting_compute_node(light_nodes_queue, node, BlockFaceDirection::TOP);
		lighting_compute_node(light_nodes_queue, node, BlockFaceDirection::BOTTOM);
		lighting_compute_node(light_nodes_queue, node, BlockFaceDirection::FRONT);
		lighting_compute_node(light_nodes_queue, node, BlockFaceDirection::BACK);
	}
}

// Expands the light from the light sources using a bfs algorithm and also sets the touching faces's light values.
void calculate_lighting(Chunk* c)
{
	TimeFunction;

	for (int i = 0; i < BLOCKS_IN_CHUNK; i++)
	{
		if (block_infos[c->blocks[i].type].is_light_emitter)
		{
			lighting_run_bfs(c, i);
		}
	}
}

void reset_lighting(Chunk* c)
{
	TimeFunction;
	static const int d = 4;
	for (int i = 0; i < BLOCKS_IN_CHUNK; i++)
	{
		Block* b = &c->blocks[i];
		if (!block_infos[b->type].is_light_emitter)
		{
			b->lighting_level[(int)BlockFaceDirection::TOP] = d;
			b->lighting_level[(int)BlockFaceDirection::BOTTOM] = d;
			b->lighting_level[(int)BlockFaceDirection::LEFT] = d;
			b->lighting_level[(int)BlockFaceDirection::RIGHT] = d;
			b->lighting_level[(int)BlockFaceDirection::FRONT] = d;
			b->lighting_level[(int)BlockFaceDirection::BACK] = d;
			b->light = d;
		}
	}
}
void chunk_generate_mesh(Chunk* chunk)
{
	TimeFunction;
	chunk->gpu_data_opaque.clear();
	chunk->gpu_data_transparent.clear();
	chunk->gpu_data_veg.clear();

	gen_mesh_opaque(chunk);
	gen_mesh_transparent(chunk);

	chunk->verts_in_use = chunk->gpu_data_opaque.size();
	chunk->verts_in_use_transparent = chunk->gpu_data_transparent.size();
	chunk->verts_in_use_veg = chunk->gpu_data_veg.size();
}

void update_buffers(GLuint vao_handle, const std::vector<GPUData>& gpu_data_arr)
{
	glBindBuffer(GL_ARRAY_BUFFER, vao_handle);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GPUData) * gpu_data_arr.size(), gpu_data_arr.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void _clear_chunk_gpu_buffers(Chunk* self)
{
	TimeFunction;
	glDeleteBuffers(1, &self->vbo_handle_opaque);
	glDeleteVertexArrays(1, &self->vao_handle_opaque);

	glDeleteBuffers(1, &self->vbo_handle_transparent);
	glDeleteVertexArrays(1, &self->vao_handle_transparent);

	glDeleteBuffers(1, &self->vbo_handle_veg);
	glDeleteVertexArrays(1, &self->vao_handle_veg);
}
#define BeginProfile();
#define EndAndPrintProfile();

void chunk_update(chunk_map_t* chunks, glm::vec3 camera_pos)
{
	BeginProfile();
	for (auto& cpair : *chunks)
	{
		if (cpair.second.needs_remesh)
		{
			reset_lighting(&cpair.second);
		}
	}

	for (auto& cpair : *chunks)
	{
		if (cpair.second.needs_remesh)
		{
			calculate_lighting(&cpair.second);
		}
	}

	for (auto& cpair : *chunks)
	{
		if (cpair.second.needs_remesh)
		{
			_clear_chunk_gpu_buffers(&cpair.second);
			chunk_generate_mesh(&cpair.second);
			chunk_generate_buffers(&cpair.second);
			cpair.second.needs_remesh = false;
		}
	}
	EndAndPrintProfile();
}

void chunk_render_opaque(Chunk* chunk, Renderer* renderer, glm::mat4 view, glm::vec3 position)
{
	if (chunk->needs_remesh)
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
	if (chunk->needs_remesh)
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
	if (chunk->needs_remesh)
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
