#include "chunk.h"

#include "glad/glad.h"

#include <glm/gtx/hash.hpp>
#include <chrono>

const int ATTRIBUTES_PER_VERTEX = 1;

static int to_1d_array(glm::ivec3 pos)
{
	return (pos.z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (pos.y * CHUNK_SIZE_WIDTH) + pos.x;
}
static int to_1d_array(short x, short y, short z)
{
	return (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x;
}

block* chunk_get_block(Chunk* c, glm::ivec3 block_pos)
{
	return &c->blocks[to_1d_array(block_pos)];
}
block* chunk_get_block(Chunk* c, short x, short y, short z)
{
	return &c->blocks[to_1d_array(x, y, z)];
}

void update_buffers(Chunk* chunk)
{
	glBindVertexArray(chunk->vao_handle);

	glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_handle);

	//glBufferSubData(GL_ARRAY_BUFFER, 0, chunk->gpu_data_used * BLOCK_SIZE_BYTES, chunk->gpu_data_arr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	_aligned_free(chunk->gpu_data_arr);
	chunk->gpu_data_arr = nullptr;

	chunk->initialized = true;
}

void init_transparent_buffers(Chunk* c)
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

void chunk_generate_buffers(Chunk* chunk)
{
	//init_transparent_buffers(chunk);

	glGenVertexArrays(1, &chunk->vao_handle);
	glBindVertexArray(chunk->vao_handle);

	glGenBuffers(1, &chunk->vbo_handle);
	glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_handle);
	glBufferData(GL_ARRAY_BUFFER, BLOCKS_IN_CHUNK * sizeof(block_size_t), chunk->gpu_data_arr, GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, ATTRIBUTES_PER_VERTEX * BLOCK_SIZE_BYTES, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int extend(block_size_t** src, int used, int amount)
{
	if (*src == nullptr)
	{
		*src = (block_size_t*)_aligned_malloc((used + amount) * BLOCK_SIZE_BYTES, 32);
	}
	else
	{
		block_size_t* more = (block_size_t*)_aligned_realloc(*src, (used + amount) * BLOCK_SIZE_BYTES, 32);
		if (more == nullptr)
		{
			assert(true);
			free(*src);
		}
		else
		{
			*src = more;
		}
	}
	return used + amount;
}

void add_face_and_texture(Chunk* chunk, const block_size_t* data, block_face_direction direction, int x, int y, int z)
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

void generate_transparent_mesh(Chunk* chunk)
{

}

void chunk_generate_mesh(Chunk* chunk)
{
	using namespace std::chrono;

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

	for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
	{
		for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
		{
			for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
			{
				int current = to_1d_array(x, y, z);
				if (chunk->blocks[current].type == BlockType::AIR)
					continue;

				//add_face_and_texture(chunk, m_back_verticies, block_face_direction::BACK, x, y, z);
				//add_face_and_texture(chunk, m_front_verticies, block_face_direction::FRONT, x, y, z);
				//add_face_and_texture(chunk, m_left_verticies, block_face_direction::LEFT, x, y, z);
				//add_face_and_texture(chunk, m_right_verticies, block_face_direction::RIGHT, x, y, z);
				//add_face_and_texture(chunk, m_bottom_verticies, block_face_direction::BOTTOM, x, y, z);
				//add_face_and_texture(chunk, m_top_verticies, block_face_direction::TOP, x, y, z);

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
			glDeleteBuffers(1, &it.second.vbo_handle);
			glDeleteVertexArrays(1, &it.second.vao_handle);
			chunk_generate_buffers(&it.second);
			it.second.dirty = false;
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
		chunk->blocks_in_use,
		position,
		glm::vec3(1));
}