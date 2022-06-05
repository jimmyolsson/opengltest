#include "chunk.h"
#include "blocks/block.h"

#include "glad/glad.h"
#include <glm/gtx/hash.hpp>

const int ATTRIBUTES_PER_VERTEX = 9;

void ChunkPrivate::update_buffers(chunk& chunk)
{
	glBindVertexArray(chunk.vao_handle);

	glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo_handle);

	glBufferSubData(GL_ARRAY_BUFFER, 0, chunk.gpu_data_used * BLOCK_SIZE_BYTES, chunk.gpu_data_arr);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	_aligned_free(chunk.gpu_data_arr);
	chunk.gpu_data_arr = nullptr;

	chunk.initialized = true;
}

void ChunkPrivate::init_buffers(chunk& chunk)
{
	glGenVertexArrays(1, &chunk.vao_handle);
	glBindVertexArray(chunk.vao_handle);

	glGenBuffers(1, &chunk.vbo_handle);
	glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo_handle);
	glBufferData(GL_ARRAY_BUFFER, chunk.gpu_data_used * sizeof(block_size_t), chunk.gpu_data_arr, GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_INT, GL_FALSE, ATTRIBUTES_PER_VERTEX * BLOCK_SIZE_BYTES, (void*)0);
	glEnableVertexAttribArray(0);

	// Normal coord
	glVertexAttribPointer(1, 3, GL_INT, GL_FALSE, ATTRIBUTES_PER_VERTEX * BLOCK_SIZE_BYTES, (void*)(3 * BLOCK_SIZE_BYTES));
	glEnableVertexAttribArray(1);

	// Blocktype
	glVertexAttribPointer(2, 1, GL_INT, GL_FALSE, ATTRIBUTES_PER_VERTEX * BLOCK_SIZE_BYTES, (void*)(6 * BLOCK_SIZE_BYTES));
	glEnableVertexAttribArray(2);

	// Texture coord
	glVertexAttribPointer(3, 2, GL_INT, GL_FALSE, ATTRIBUTES_PER_VERTEX * BLOCK_SIZE_BYTES, (void*)(7 * BLOCK_SIZE_BYTES));
	glEnableVertexAttribArray(3);

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

#include <immintrin.h>
#include <chrono>

static int to_1d_array(int x, int y, int z)
{
	return (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x;
}

void add_face_and_texture(chunk& chunk, const block_size_t* data, block_face_direction direction, int x, int y, int z)
{
	const int vert_count = 48;
	//const __m256i vals = _mm256_set_epi32(0, 0, 0, 0, 0, z, y, x);
	//for (int a = 0; a < vert_count; a += 8)
	//{
	//	__m256i vert_data = _mm256_loadu_epi32(data + a);
	//	__m256i result = _mm256_add_epi32(vals, vert_data);

	//	// this might need a memory fence
	//	_mm256_storeu_si256((__m256i*) & chunk.gpu_data_arr[chunk.gpu_data_used + a], result);
	//}
	//chunk.gpu_data_used += 48;

	int i = 0;
	while (i != vert_count)
	{
		chunk.gpu_data_arr[chunk.gpu_data_used] = data[i] + x;
		chunk.gpu_data_arr[chunk.gpu_data_used + 1] = data[i + 1] + y;
		chunk.gpu_data_arr[chunk.gpu_data_used + 2] = data[i + 2] + z;

		chunk.gpu_data_arr[chunk.gpu_data_used + 3] = data[i + 3];
		chunk.gpu_data_arr[chunk.gpu_data_used + 4] = data[i + 4];
		chunk.gpu_data_arr[chunk.gpu_data_used + 5] = data[i + 5];

		chunk.gpu_data_arr[chunk.gpu_data_used + 6] = block_get_texture(direction, chunk.blocks[to_1d_array(x, y, z)].type);

		chunk.gpu_data_arr[chunk.gpu_data_used + 7] = data[i + 6];
		chunk.gpu_data_arr[chunk.gpu_data_used + 8] = data[i + 7];

		// For next pass
		chunk.gpu_data_used += 9;
		i += 8;
	}
}

//void generate_face(chunk& current_chunk, const glm::ivec2& neighbor_chunk_pos, const float data[30], block_face_direction direction, int x, int y, int z, int other_chunk_index, int current_chunk_index, bool on_edge)
void generate_face(chunk& current_chunk, const chunk* neighbor, const block_size_t data[30], block_face_direction direction, int x, int y, int z, int other_chunk_index, int current_chunk_index, bool on_edge)
{
	if (on_edge)
	{
		//Check if the block(that belongs to another chunk) is occupied
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

float* generate_indexes_simd(int x, int y)
{
	const __m256 vheight = _mm256_set1_ps(CHUNK_SIZE_HEIGHT);
	const __m256 vwidth = _mm256_set1_ps(CHUNK_SIZE_WIDTH);

	const __m256 xindex = _mm256_set1_ps(x);
	const __m256 yindex = _mm256_set1_ps(y);

	__m256 start_zindex = _mm256_set_ps(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0);
	__m256 increment = _mm256_set1_ps(8);

	float indexes[CHUNK_SIZE_WIDTH];

	for (int z = 0; z < CHUNK_SIZE_WIDTH; z += 8)
	{
		start_zindex = _mm256_add_ps(increment, start_zindex);
		{
			// to_1d_array(x, y, z)
			__m256 zindex = start_zindex;

			// z * CHUNK_SIZE_WIDTH
			__m256 lside = _mm256_mul_ps(zindex, vwidth);
			// (z * CHUNK_SIZE_WIDTH) * CHUNK_SIZE_HEIGHT
			__m256 lside2 = _mm256_mul_ps(lside, vheight);
			// y * CHUNK_SIZE_WIDTH + x
			__m256 rside = _mm256_fmaddsub_ps(yindex, vwidth, xindex);
			// (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x
			__m256 flat_index = _mm256_add_ps(lside2, rside);

			//_mm256_store_ps(indexes + z, flat_index);
			_mm256_stream_ps(indexes + z, flat_index);
		}
		{
			// to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1)
			__m256 zindex = _mm256_sub_ps(vwidth, _mm256_set1_ps(1.0));

			// z * CHUNK_SIZE_WIDTH
			__m256 lside = _mm256_mul_ps(zindex, vwidth);
			// (z * CHUNK_SIZE_WIDTH) * CHUNK_SIZE_HEIGHT
			__m256 lside2 = _mm256_mul_ps(lside, vheight);
			// y * CHUNK_SIZE_WIDTH + x
			__m256 rside = _mm256_fmaddsub_ps(yindex, vwidth, xindex);
			// (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x
			__m256 flat_index = _mm256_add_ps(lside2, rside);

			//_mm256_store_ps(indexes + z, flat_index);
			_mm256_stream_ps(indexes + z, flat_index);
		}
		{
			// to_1d_array(x, y, z - 1)
			__m256 zindex = _mm256_sub_ps(start_zindex, _mm256_set1_ps(1.0));

			// z * CHUNK_SIZE_WIDTH
			__m256 lside = _mm256_mul_ps(zindex, vwidth);
			// (z * CHUNK_SIZE_WIDTH) * CHUNK_SIZE_HEIGHT
			__m256 lside2 = _mm256_mul_ps(lside, vheight);
			// y * CHUNK_SIZE_WIDTH + x
			__m256 rside = _mm256_fmaddsub_ps(yindex, vwidth, xindex);
			// (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x
			__m256 flat_index = _mm256_add_ps(lside2, rside);

			//_mm256_store_ps(indexes + z, flat_index);
			_mm256_stream_ps(indexes + z, flat_index);
		}
		{
			// to_1d_array(x, y, 0)
			__m256 zindex = _mm256_set1_ps(0.0);

			// z * CHUNK_SIZE_WIDTH
			__m256 lside = _mm256_mul_ps(zindex, vwidth);
			// (z * CHUNK_SIZE_WIDTH) * CHUNK_SIZE_HEIGHT
			__m256 lside2 = _mm256_mul_ps(lside, vheight);
			// y * CHUNK_SIZE_WIDTH + x
			__m256 rside = _mm256_fmaddsub_ps(yindex, vwidth, xindex);
			// (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x
			__m256 flat_index = _mm256_add_ps(lside2, rside);

			//_mm256_store_ps(indexes + z, flat_index);
			_mm256_stream_ps(indexes + z, flat_index);
		}
		{
			// to_1d_array(x, y, z + 1)
			__m256 zindex = _mm256_add_ps(start_zindex, _mm256_set1_ps(1.0));
			zindex = _mm256_sub_ps(zindex, _mm256_set1_ps(1.0));

			// z * CHUNK_SIZE_WIDTH
			__m256 lside = _mm256_mul_ps(zindex, vwidth);
			// (z * CHUNK_SIZE_WIDTH) * CHUNK_SIZE_HEIGHT
			__m256 lside2 = _mm256_mul_ps(lside, vheight);
			// y * CHUNK_SIZE_WIDTH + x
			__m256 rside = _mm256_fmaddsub_ps(yindex, vwidth, xindex);
			// (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x
			__m256 flat_index = _mm256_add_ps(lside2, rside);

			//_mm256_store_ps(indexes + z, flat_index);
			_mm256_stream_ps(indexes + z, flat_index);
		}
		{
			// to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z)
			__m256 zindex = start_zindex;

			// z * CHUNK_SIZE_WIDTH
			__m256 lside = _mm256_mul_ps(zindex, vwidth);
			// (z * CHUNK_SIZE_WIDTH) * CHUNK_SIZE_HEIGHT
			__m256 lside2 = _mm256_mul_ps(lside, vheight);
			// y * CHUNK_SIZE_WIDTH + x
			__m256 xindex2 = _mm256_sub_ps(vwidth, _mm256_set1_ps(1.0));
			__m256 rside = _mm256_fmaddsub_ps(yindex, vwidth, xindex2);
			// (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x
			__m256 flat_index = _mm256_add_ps(lside2, rside);

			//_mm256_store_ps(indexes + z, flat_index);
			_mm256_stream_ps(indexes + z, flat_index);
		}
		{
			// to_1d_array(x - 1, y, z)
			__m256 zindex = start_zindex;

			// z * CHUNK_SIZE_WIDTH
			__m256 lside = _mm256_mul_ps(zindex, vwidth);
			// (z * CHUNK_SIZE_WIDTH) * CHUNK_SIZE_HEIGHT
			__m256 lside2 = _mm256_mul_ps(lside, vheight);
			// y * CHUNK_SIZE_WIDTH + x
			__m256 rside = _mm256_fmaddsub_ps(yindex, vwidth, xindex);
			// (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x
			__m256 flat_index = _mm256_add_ps(lside2, rside);

			//_mm256_store_ps(indexes + z, flat_index);
			_mm256_stream_ps(indexes + z, flat_index);
		}
	}

	return indexes;
}


void ChunkPrivate::generate_mesh(chunk& chunk, const glm::vec2& chunk_pos)
{
	int triangle_count = 0;
	const int alloc_size = 1024 * 1024 * 10;
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

	chunk.gpu_data_length = extend(&chunk.gpu_data_arr, chunk.gpu_data_used, alloc_size);

	constexpr int32_t STEP_X = 1;
	constexpr int32_t STEP_Y = CHUNK_SIZE_HEIGHT;
	constexpr int32_t STEP_Z = CHUNK_SIZE_WIDTH * CHUNK_SIZE_WIDTH;

	int32_t noiseIdx = STEP_X + STEP_Y + STEP_Z;

	for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
	{
		for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
		{
			for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
			{
				int current = to_1d_array(x, y, z);
				if (chunk.blocks[current].type == block_type::AIR || chunk.blocks[current].type == block_type::WATER)
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

				if (chunk.gpu_data_used > chunk.gpu_data_length - 2048)
					chunk.gpu_data_length = extend(&chunk.gpu_data_arr, chunk.gpu_data_used, alloc_size);

				noiseIdx++;
			}

			noiseIdx += STEP_X * 2;
		}

		noiseIdx += STEP_Y * 2;
	}

	chunk.gpu_data_length = 0;
}

void ChunkPrivate::draw(const chunk& chunk)
{
	glBindVertexArray(chunk.vao_handle);
	glDrawArrays(GL_TRIANGLES, 0, chunk.gpu_data_used / ATTRIBUTES_PER_VERTEX);
}