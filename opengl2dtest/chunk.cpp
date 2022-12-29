#include "chunk.h"

#include "glad/glad.h"

#include "util/common.h"

#include <glm/gtx/hash.hpp>
#include <chrono>

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
	glBufferData(GL_ARRAY_BUFFER, BLOCKS_IN_CHUNK * sizeof(block_size_t), c->gpu_data_arr_transparent, GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, ATTRIBUTES_PER_VERTEX * BLOCK_SIZE_BYTES, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

static const int gpub = (((sizeof(int) * 6) * 6) * BLOCKS_IN_CHUNK);

void update_buffers(Chunk* chunk)
{
	glBindVertexArray(chunk->vao_handle);
	glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_handle);

	glBufferSubData(GL_ARRAY_BUFFER, 0, gpub, chunk->gpu_data_arr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void chunk_generate_buffers(Chunk* chunk)
{
	glGenVertexArrays(1, &chunk->vao_handle);
	glBindVertexArray(chunk->vao_handle);

	glGenBuffers(1, &chunk->vbo_handle);
	glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_handle);
	glBufferData(GL_ARRAY_BUFFER, gpub, chunk->gpu_data_arr, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, ATTRIBUTES_PER_VERTEX * BLOCK_SIZE_BYTES, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int extend(block_size_t** src, int used, int amount)
{
	const int kAlignment = 32;

	// Allocate memory for the array, or resize it if it already exists.
	block_size_t* more = (block_size_t*)_aligned_realloc(*src, (used + amount) * BLOCK_SIZE_BYTES, kAlignment);
	if (more == nullptr)
	{
		// Failed to allocate/resize the array. Handle the error here.
		return -1;
	}

	// Update the pointer to the array and return the new number of elements.
	*src = more;
	return used + amount;
}

// The pos argument assumes that you are 'looking' at the back-face
void find_inc(Chunk* chunk, int i, block_face_direction direction, glm::vec3 block_pos, short* occlusion, glm::vec3 pos)
{
	glm::vec3 a = block_pos + pos;
	block* b = chunk_get_block(chunk, a);
	if (b != nullptr)
		if (!(b->type == BlockType::AIR || b->type == BlockType::WATER))
			*occlusion += 1;
}

void calc_ac(Chunk* chunk, int i, block_face_direction direction, glm::vec3 block_pos, short* occlusion)
{
	// Top left
	if (i == 0)
	{
		if (direction == block_face_direction::BACK)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(0, 1, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 0, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 1, -1));
		}
		else if (direction == block_face_direction::FRONT)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 1, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 0, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(0, 1, 1));
		}
		else if (direction == block_face_direction::TOP)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 1, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 1, 0));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(0, 1, 1));
		}
		else if (direction == block_face_direction::RIGHT)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 1, 0));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 0, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 1, 1));
		}
		else if (direction == block_face_direction::LEFT)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 1, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 0, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 1, 0));
		}
		else if (direction == block_face_direction::BOTTOM)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 0, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(0, 0, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 0, 1));
		}
	}
	// Top right
	else if (i == 10 || i == 25)
	{
		if (direction == block_face_direction::BACK)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 1, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 0, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(0, 1, -1));
		}
		else if (direction == block_face_direction::FRONT)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(0, 1, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 0, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 1, 1));
		}
		else if (direction == block_face_direction::TOP)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 1, 0));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 1, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(0, 1, -1));
		}
		else if (direction == block_face_direction::RIGHT)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 1, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 0, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 1, 0));
		}
		else if (direction == block_face_direction::LEFT)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 1, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 0, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 1, 0));
		}

	}
	// Bottom left
	else if (i == 5 || i == 15)
	{
		if (direction == block_face_direction::BACK)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(0, -1, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 0, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, -1, -1));
		}
		else if (direction == block_face_direction::FRONT)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, -1, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 0, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(0, -1, 1));
		}
		else if (direction == block_face_direction::TOP)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(0, 1, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 1, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 1, 0));
		}
		else if (direction == block_face_direction::RIGHT)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, -1, 0));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 0, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, -1, 1));
		}
		else if (direction == block_face_direction::LEFT)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, -1, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 0, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, -1, 0));
		}
	}
	// Bottom right
	else if (i == 20)
	{
		if (direction == block_face_direction::BACK)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, -1, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 0, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(0, -1, -1));
		}
		else if (direction == block_face_direction::FRONT)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(0, -1, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 0, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, -1, 1));
		}
		else if (direction == block_face_direction::TOP)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(0, 1, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 1, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 1, 0));
		}
		else if (direction == block_face_direction::RIGHT)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, -1, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, 0, -1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(1, -1, 0));
		}
		else if (direction == block_face_direction::LEFT)
		{
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, -1, 0));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, 0, 1));
			find_inc(chunk, i, direction, block_pos, occlusion, glm::vec3(-1, -1, 1));
		}
	}
}
void add_face_and_texture(Chunk* chunk, const block_size_t* data, block_face_direction direction, int x, int y, int z)
{
	glm::vec3 block_pos = glm::vec3(x, y, z);

	const int vert_count = 30;
	int i = 0;
	while (i != vert_count)
	{
		// xxxxxxxx yyyyyyyy zzzzzzzz uvaaaaaa
		short occlusion = 0;
		calc_ac(chunk, i, direction, block_pos, &occlusion);
		int result = 0;
		result = (data[i] + x) << 24;								//x
		result |= (data[i + 1] + y) << 16;							//y
		result |= (data[i + 2] + z) << 8;							//z
		result |= (data[i + 3]) << 7;								//u
		result |= (data[i + 4]) << 6;								//v
		result |= (block_get_texture(direction, chunk->blocks[to_1d_array(x, y, z)].type)) << 2;		//b
		result |= occlusion;

		chunk->gpu_data_arr[chunk->blocks_in_use] = result;

		chunk->blocks_in_use += ATTRIBUTES_PER_VERTEX;
		i += 5;
	}
}

void generate_face(Chunk* current_chunk, const Chunk* neighbor, const block_size_t data[30], block_face_direction direction, int x, int y, int z, int other_chunk_index, int current_chunk_index, bool on_edge)
{
	if (on_edge)
	{
		//If i have a neighbor that is transparent
		if (neighbor != nullptr && block_is_transparent(neighbor->blocks[other_chunk_index].type))
		{
			add_face_and_texture(current_chunk, data, direction, x, y, z);
		}
	}
	// Is the block next to me occupied?
	else if (block_is_transparent(current_chunk->blocks[current_chunk_index].type))
	{
		add_face_and_texture(current_chunk, data, direction, x, y, z);
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

void check_neighbors(Chunk* chunk)
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

void add_face_and_texture_t(Chunk* chunk, const block_size_t* data, block_face_direction direction, int x, int y, int z)
{
	const int vert_count = 30;
	int i = 0;
	while (i != vert_count)
	{
		int result = 0;
		result = (data[i] + x) << 24;								//x
		result |= (data[i + 1] + y) << 16;							//y
		result |= (data[i + 2] + z) << 8;							//z
		result |= (data[i + 3]) << 7;								//u
		result |= (data[i + 4]) << 6;								//v
		result |= (block_get_texture(direction, chunk->blocks[to_1d_array(x, y, z)].type)) << 2;		//b
		result |= (direction == block_face_direction::RIGHT || direction == block_face_direction::LEFT) ? 2 : 1; // basic lighting

		chunk->gpu_data_arr_transparent[chunk->blocks_in_use_transparent] = result;

		chunk->blocks_in_use_transparent += ATTRIBUTES_PER_VERTEX;
		i += 5;
	}
}

void generate_face_t(Chunk* current_chunk, const Chunk* neighbor, const block_size_t data[30], block_face_direction direction, int x, int y, int z, int other_chunk_index, int current_chunk_index, bool on_edge)
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

void chunk_generate_mesh_transparent(Chunk* chunk)
{
	check_neighbors(chunk);

	for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
	{
		for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
		{
			for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
			{
				int current = to_1d_array(x, y, z);
				if (chunk->blocks[current].type != BlockType::WATER)
					continue;

				generate_face_t(chunk, chunk->back_neighbor, m_back_verticies, block_face_direction::BACK, x, y, z, to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1), to_1d_array(x, y, z - 1), z == 0);
				generate_face_t(chunk, chunk->front_neighbor, m_front_verticies, block_face_direction::FRONT, x, y, z, to_1d_array(x, y, 0), to_1d_array(x, y, z + 1), (z + 1) >= CHUNK_SIZE_WIDTH);

				generate_face_t(chunk, chunk->left_neighbor, m_left_verticies, block_face_direction::LEFT, x, y, z, to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z), to_1d_array(x - 1, y, z), x == 0);
				generate_face_t(chunk, chunk->right_neighbor, m_right_verticies, block_face_direction::RIGHT, x, y, z, to_1d_array(0, y, z), to_1d_array(x + 1, y, z), ((x + 1) >= CHUNK_SIZE_WIDTH));

				// no chunk neighbors on the Y-axis
				if (y != 0 && (y == 0 || chunk->blocks[to_1d_array(x, y - 1, z)].type != BlockType::WATER))
				{
					add_face_and_texture_t(chunk, m_bottom_verticies, block_face_direction::BOTTOM, x, y, z);
				}

				if (y + 1 > CHUNK_SIZE_HEIGHT || chunk->blocks[to_1d_array(x, y + 1, z)].type == BlockType::AIR)
				{
					add_face_and_texture_t(chunk, m_top_verticies, block_face_direction::TOP, x, y, z);
				}
			}
		}
	}
}

// TODO: Implement "i know that im ur neighbor" - technique
void chunk_generate_mesh(Chunk* chunk)
{
	using namespace std::chrono;
	check_neighbors(chunk);
	TIMER_START(ORIGINAL_MESHING);
	for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
	{
		for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
		{
			for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
			{
				int current = to_1d_array(x, y, z);

				if (chunk->blocks[current].type == BlockType::AIR || chunk->blocks[current].type == BlockType::WATER)
					continue;

				generate_face(chunk, chunk->back_neighbor, m_back_verticies, block_face_direction::BACK, x, y, z, to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1), to_1d_array(x, y, z - 1), z == 0);
				generate_face(chunk, chunk->front_neighbor, m_front_verticies, block_face_direction::FRONT, x, y, z, to_1d_array(x, y, 0), to_1d_array(x, y, z + 1), (z + 1) >= CHUNK_SIZE_WIDTH);

				generate_face(chunk, chunk->left_neighbor, m_left_verticies, block_face_direction::LEFT, x, y, z, to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z), to_1d_array(x - 1, y, z), x == 0);
				generate_face(chunk, chunk->right_neighbor, m_right_verticies, block_face_direction::RIGHT, x, y, z, to_1d_array(0, y, z), to_1d_array(x + 1, y, z), ((x + 1) >= CHUNK_SIZE_WIDTH));

				// no chunk neighbors on the Y-axis
				if (y != 0 && (y == 0 || block_is_transparent(chunk->blocks[to_1d_array(x, y - 1, z)].type)))
				{
					add_face_and_texture(chunk, m_bottom_verticies, block_face_direction::BOTTOM, x, y, z);
				}

				if (y + 1 > CHUNK_SIZE_HEIGHT || block_is_transparent(chunk->blocks[to_1d_array(x, y + 1, z)].type))
				{
					add_face_and_texture(chunk, m_top_verticies, block_face_direction::TOP, x, y, z);
				}
			}
		}
	}
	TIMER_END(ORIGINAL_MESHING);
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

//TODO: Multithread this
void chunk_update(chunk_map_t* chunks)
{
	for (decltype(auto) it : *chunks)
	{
		if (it.second.dirty)
		{
			chunk_generate_mesh(&it.second);
			update_buffers(&it.second);
			it.second.dirty = false;
		}
	}
}

void chunk_render(Chunk* chunk, Renderer* renderer, glm::mat4 view, glm::vec3 position, int enabled)
{
	if (chunk->dirty)
		return;

	renderer_render_custom(renderer,
		view,
		TEXTURE_ATLAS_CHUNK,
		SHADER_CHUNK,
		chunk->vao_handle,
		chunk->blocks_in_use,
		position,
		glm::vec3(1),
		enabled);
}

void chunk_render_transparent(Chunk* chunk, Renderer* renderer, glm::mat4 view, glm::vec3 position, int enabled)
{
	if (chunk->dirty)
		return;

	renderer_render_custom(renderer,
		view,
		TEXTURE_ATLAS_CHUNK,
		SHADER_CHUNK,
		chunk->vao_handle_transparent,
		chunk->blocks_in_use_transparent,
		position,
		glm::vec3(1),
		enabled);
}
