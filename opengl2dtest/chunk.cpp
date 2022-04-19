#include "chunk.h"
#include "blocks/block.h"

#include "glad/glad.h"
#include <glm/gtx/hash.hpp>

void ChunkPrivate::update_buffers(chunk& chunk)
{
	if (!chunk.initialized)
	{
		init_buffers(chunk);
	}
	else if (chunk.gpu_data_used > chunk.gpu_data_last_used || !chunk.gpu_data_used)
	{
		glBindVertexArray(chunk.vao_handle);
		glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo_handle);

		glBufferData(GL_ARRAY_BUFFER, chunk.gpu_data_used * BLOCK_SIZE_BYTES, chunk.gpu_data_arr, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	else
	{
		glBindVertexArray(chunk.vao_handle);

		glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo_handle);
		
		glBufferSubData(GL_ARRAY_BUFFER, 0, chunk.gpu_data_used * BLOCK_SIZE_BYTES, chunk.gpu_data_arr);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	free(chunk.gpu_data_arr);
	chunk.gpu_data_arr = nullptr;

	chunk.initialized = true;
}

void ChunkPrivate::init_buffers(chunk& chunk)
{
	glGenVertexArrays(1, &chunk.vao_handle);
	glBindVertexArray(chunk.vao_handle);

	glGenBuffers(1, &chunk.vbo_handle);
	glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo_handle);
	glBufferData(GL_ARRAY_BUFFER, chunk.gpu_data_used * BLOCK_SIZE_BYTES, chunk.gpu_data_arr, GL_STATIC_DRAW);

	// Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_SHORT, GL_FALSE, 6 * BLOCK_SIZE_BYTES, (void*)0);

	// Texture coord
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_SHORT, GL_FALSE, 6 * BLOCK_SIZE_BYTES, (void*)(3 * BLOCK_SIZE_BYTES));

	// Blocktype
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_SHORT, GL_FALSE, 6 * BLOCK_SIZE_BYTES, (void*)(5 * BLOCK_SIZE_BYTES));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int extend(block_size_t** src, int used, int amount)
{
	if (*src == nullptr)
	{
		*src = (block_size_t*)malloc((used + amount) * BLOCK_SIZE_BYTES);
	}
	else
	{
		block_size_t* more = (block_size_t*)realloc(*src, (used + amount) * BLOCK_SIZE_BYTES);
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

void add_face_and_texture(chunk& chunk, const block_size_t* data, block_face_direction direction, int x, int y, int z)
{
	const int vert_count = 30;
	int i = 0;
	while (i != vert_count)
	{
		chunk.gpu_data_arr[chunk.gpu_data_used] = data[i] + x;

		chunk.gpu_data_arr[chunk.gpu_data_used + 1] = data[i + 1] + y;

		chunk.gpu_data_arr[chunk.gpu_data_used + 2] = data[i + 2] + z;

		chunk.gpu_data_arr[chunk.gpu_data_used + 3] = data[i + 3];

		chunk.gpu_data_arr[chunk.gpu_data_used + 4] = data[i + 4];

		chunk.gpu_data_arr[chunk.gpu_data_used + 5] = block_get_texture(direction, chunk.blocks[to_1d_array(x, y, z)].type);

		// For next pass
		chunk.gpu_data_used += 6;
		i += 5;
	}
}

//void generate_face(chunk& current_chunk, const glm::ivec2& neighbor_chunk_pos, const float data[30], block_face_direction direction, int x, int y, int z, int other_chunk_index, int current_chunk_index, bool on_edge)
void generate_face(chunk& current_chunk, const chunk* neighbor, const block_size_t data[30], block_face_direction direction, int x, int y, int z, int other_chunk_index, int current_chunk_index, bool on_edge)
{
	if (on_edge)
	{
		// Check if the block(that belongs to another chunk) is occupied
		if (neighbor != nullptr && neighbor->blocks[other_chunk_index].type == block_type::AIR)
		{
			add_face_and_texture(current_chunk, data, direction, x, y, z);
		}
	}
	// Is the block next to me occupied?
	else if (current_chunk.blocks[current_chunk_index].type == block_type::AIR)
	{
		add_face_and_texture(current_chunk, data, direction, x, y, z);
	}
}

void ChunkPrivate::generate_mesh(chunk& chunk, const glm::vec2& chunk_pos)
{
	using namespace ChunkPrivate;

	if (chunk.back_neighbor == nullptr && chunk.chunks->contains(glm::ivec2((int)chunk_pos.x, (int)chunk_pos.y - CHUNK_SIZE_WIDTH)))
	{
		chunk.back_neighbor = &chunk.chunks->at(glm::ivec2((int)chunk_pos.x, (int)chunk_pos.y - CHUNK_SIZE_WIDTH));
	}
	if (chunk.front_neighbor == nullptr && chunk.chunks->contains(glm::ivec2((int)chunk_pos.x, (int)chunk_pos.y + CHUNK_SIZE_WIDTH)))
	{
		chunk.front_neighbor = &chunk.chunks->at(glm::ivec2((int)chunk_pos.x, (int)chunk_pos.y + CHUNK_SIZE_WIDTH));
	}
	if (chunk.left_neighbor == nullptr && chunk.chunks->contains(glm::ivec2((int)chunk_pos.x - CHUNK_SIZE_WIDTH, (int)chunk_pos.y)))
	{
		chunk.left_neighbor = &chunk.chunks->at(glm::ivec2((int)chunk_pos.x - CHUNK_SIZE_WIDTH, (int)chunk_pos.y));
	}
	if (chunk.right_neighbor == nullptr && chunk.chunks->contains(glm::ivec2((int)chunk_pos.x + CHUNK_SIZE_WIDTH, (int)chunk_pos.y)))
	{
		chunk.right_neighbor = &chunk.chunks->at(glm::ivec2((int)chunk_pos.x + CHUNK_SIZE_WIDTH, (int)chunk_pos.y));
	}

	chunk.gpu_data_last_used = chunk.gpu_data_used;
	chunk.gpu_data_used = 0;

	chunk.gpu_data_length = extend(&chunk.gpu_data_arr, chunk.gpu_data_used, 32768);

	for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
	{
		for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
		{
			for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
			{
				int vert_count = 30;
				if (chunk.blocks[to_1d_array(x, y, z)].type == block_type::AIR)
					continue;

				//add_face_and_texture(chunk, m_back_verticies, block_face_direction::BACK, x, y, z);
				//add_face_and_texture(chunk, m_front_verticies, block_face_direction::FRONT, x, y, z);
				//add_face_and_texture(chunk, m_left_verticies, block_face_direction::LEFT, x, y, z);
				//add_face_and_texture(chunk, m_right_verticies, block_face_direction::RIGHT, x, y, z);
				//add_face_and_texture(chunk, m_bottom_verticies, block_face_direction::BOTTOM, x, y, z);
				//add_face_and_texture(chunk, m_top_verticies, block_face_direction::TOP, x, y, z);

				generate_face(chunk, chunk.back_neighbor, m_back_verticies, block_face_direction::BACK, x, y, z, to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1), to_1d_array(x, y, z - 1), z == 0);
				generate_face(chunk, chunk.front_neighbor, m_front_verticies, block_face_direction::FRONT, x, y, z, to_1d_array(x, y, 0), to_1d_array(x, y, z + 1), (z + 1) >= CHUNK_SIZE_WIDTH);

				generate_face(chunk, chunk.left_neighbor, m_left_verticies, block_face_direction::LEFT, x, y, z, to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z), to_1d_array(x - 1, y, z), x == 0);
				generate_face(chunk, chunk.right_neighbor, m_right_verticies, block_face_direction::RIGHT, x, y, z, to_1d_array(0, y, z), to_1d_array(x + 1, y, z), ((x + 1) >= CHUNK_SIZE_WIDTH));

				//bottom
				if (y != 0 && (y == 0 || chunk.blocks[to_1d_array(x, y - 1, z)].type == block_type::AIR))
				{
					add_face_and_texture(chunk, m_bottom_verticies, block_face_direction::BOTTOM, x, y, z);
				}
				else if (((y + 1) >= CHUNK_SIZE_HEIGHT) || chunk.blocks[to_1d_array(x, y + 1, z)].type == block_type::AIR)
				{
					add_face_and_texture(chunk, m_top_verticies, block_face_direction::TOP, x, y, z);
				}

				if (chunk.gpu_data_used > chunk.gpu_data_length - 4096)
					chunk.gpu_data_length = extend(&chunk.gpu_data_arr, chunk.gpu_data_used, 32768);
			}
		}
	}

	chunk.gpu_data_length = 0;
}

void ChunkPrivate::draw(chunk& chunk)
{
	glBindVertexArray(chunk.vao_handle);
	glDrawArrays(GL_TRIANGLES, 0, chunk.gpu_data_used/6);
}