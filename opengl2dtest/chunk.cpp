#include "chunk.h"

#include <glad/glad.h>
#include <glm/gtx/hash.hpp>
#include "blocks/block.h"

void ChunkPrivate::init_buffers(chunk& chunk, int& counter)
{
	glGenVertexArrays(1, &chunk.vao_handle);
	glBindVertexArray(chunk.vao_handle);

	glGenBuffers(1, &chunk.vbo_handle);
	glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo_handle);
	//glBufferData(GL_ARRAY_BUFFER, chunk.gpu_data.size() * sizeof(float), chunk.gpu_data.data(), GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, chunk.gpu_data_used * sizeof(float), chunk.gpu_data_arr, GL_STATIC_DRAW);

	// Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

	// Texture coord
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	// Blocktype
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	counter += chunk.gpu_data_length * sizeof(float);

	//delete[] chunk.gpu_data_arr;
	//chunk.gpu_data_arr = nullptr;
}

int extend(float** src, int used, int amount, int new_length)
{
	float* temp = new float[used + amount];

	if (*src != nullptr)
	{
		std::copy(*src, *src + used, temp);
		delete[] * src;
		*src = nullptr;
	}
	*src = temp;

	return used + amount;
}

void add_face_and_texture(chunk& chunk, const float* data, block_face_direction direction, int x, int y, int z)
{
	int vert_count = 30;
	int i = 0;
	while (i != vert_count)
	{
		chunk.gpu_data_arr[chunk.gpu_data_used] = data[i] + x;

		i++;
		chunk.gpu_data_used++;
		chunk.gpu_data_arr[chunk.gpu_data_used] = data[i] + y;

		i++;
		chunk.gpu_data_used++;
		chunk.gpu_data_arr[chunk.gpu_data_used] = data[i] + z;

		i++;
		chunk.gpu_data_used++;
		chunk.gpu_data_arr[chunk.gpu_data_used] = data[i];

		i++;
		chunk.gpu_data_used++;
		chunk.gpu_data_arr[chunk.gpu_data_used] = data[i];

		i++;
		chunk.gpu_data_used++;
		chunk.gpu_data_arr[chunk.gpu_data_used] = block_get_texture(direction, chunk.blocks[to_1d_array(x, y, z)].type);

		// For next pass
		chunk.gpu_data_used++;
	}
}


void generate_face(chunk& chunk, const glm::ivec2& neighbor_chunk_pos, const float data[30], block_face_direction direction, int x, int y, int z, int other_chunk_index, int current_chunk_index, bool on_edge)
{
	int vert_count = 30;
	if (on_edge)
	{
		// Is it a chunk next to me?
		if (chunk.chunks->contains(neighbor_chunk_pos))
		{
			// Check if the block(that belongs to another chunk) is occupied
			if (chunk.chunks->at(neighbor_chunk_pos).blocks[other_chunk_index].type == block_type::AIR)
			{
				add_face_and_texture(chunk, data, direction, x, y, z);
			}
		}
	}
	// Is the block next to me occupied?
	else if (chunk.blocks[current_chunk_index].type == block_type::AIR)
	{
		add_face_and_texture(chunk, data, direction, x, y, z);
	}
}

void ChunkPrivate::generate_mesh(chunk& chunk, const glm::vec2& chunk_pos)
{
	using namespace ChunkPrivate;

	chunk.gpu_data_used = 0;
	chunk.gpu_data_length = 8192;
	chunk.gpu_data_length = extend(&chunk.gpu_data_arr, chunk.gpu_data_used, 8192, chunk.gpu_data_length);

	for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
	{
		for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
		{
			for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
			{
				int vert_count = 30;
				if (chunk.blocks[to_1d_array(x, y, z)].type == block_type::AIR)
					continue;

				generate_face(chunk, glm::ivec2((int)chunk_pos.x, (int)chunk_pos.y - CHUNK_SIZE_WIDTH), m_back_verticies, block_face_direction::BACK, x, y, z, to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1), to_1d_array(x, y, z - 1), z == 0);
				generate_face(chunk, glm::ivec2((int)chunk_pos.x, (int)chunk_pos.y + CHUNK_SIZE_WIDTH), m_front_verticies, block_face_direction::FRONT, x, y, z, to_1d_array(x, y, 0), to_1d_array(x, y, z + 1), (z + 1) >= CHUNK_SIZE_WIDTH);

				generate_face(chunk, glm::ivec2((int)chunk_pos.x - CHUNK_SIZE_WIDTH, (int)chunk_pos.y), m_left_verticies, block_face_direction::LEFT, x, y, z, to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z), to_1d_array(x - 1, y, z), x == 0);
				generate_face(chunk, glm::ivec2((int)chunk_pos.x + CHUNK_SIZE_WIDTH, (int)chunk_pos.y), m_right_verticies, block_face_direction::RIGHT, x, y, z, to_1d_array(0, y, z), to_1d_array(x + 1, y, z), ((x + 1) >= CHUNK_SIZE_WIDTH));

				//bottom
				if (y != 0 && (y == 0 || chunk.blocks[to_1d_array(x, y - 1, z)].type == block_type::AIR))
				{
					add_face_and_texture(chunk, m_bottom_verticies, block_face_direction::BOTTOM, x, y, z);
				}
				// top
				if (((y + 1) >= CHUNK_SIZE_HEIGHT) || chunk.blocks[to_1d_array(x, y + 1, z)].type == block_type::AIR)
				{
					add_face_and_texture(chunk, m_top_verticies, block_face_direction::TOP, x, y, z);
				}

				if (chunk.gpu_data_used > chunk.gpu_data_length - 4096)
					chunk.gpu_data_length = extend(&chunk.gpu_data_arr, chunk.gpu_data_used, 8192, chunk.gpu_data_length);
			}
		}
	}
}

void ChunkPrivate::draw(chunk& chunk)
{
	glBindVertexArray(chunk.vao_handle);
	glDrawArrays(GL_TRIANGLES, 0, chunk.gpu_data_used / 6);
}