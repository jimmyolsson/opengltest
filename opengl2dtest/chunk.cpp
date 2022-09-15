#include "chunk.h"
#include "blocks/block.h"

#include "glad/glad.h"
#include <glm/gtx/hash.hpp>

const int ATTRIBUTES_PER_VERTEX = 1;

void chunk_render(chunk* chunk)
{

}

void ChunkPrivate::update_buffers(chunk* chunk)
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

void init_transparent_buffers(chunk* c)
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

void ChunkPrivate::init_buffers(chunk* chunk)
{
	init_transparent_buffers(chunk);

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

#include <immintrin.h>
#include <chrono>

static int to_1d_array(int x, int y, int z)
{
	return (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x;
}

static int get_block_at(int x, int y, int z)
{
	return (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x;
}

void add_face_and_texture(chunk* chunk, const block_size_t* data, block_face_direction direction, int x, int y, int z)
{
	const int vert_count = 30;
	//const __m256i vals = _mm256_set_epi32(0, 0, 0, 0, 0, z, y, x);
	//for (int a = 0; a < vert_count; a += 9)
	//{
	//	__m256i vert_data = _mm256_loadu_epi32(data + a);
	//	__m256i result = _mm256_add_epi32(vals, vert_data);

	//	// this might need a memory fence
	//	_mm256_storeu_si256((__m256i*) & chunk->gpu_data_arr[chunk->gpu_data_used + a], result);
	//	chunk->gpu_data_arr[chunk->gpu_data_used + a + 1] = block_get_texture(direction, chunk->blocks[to_1d_array(x, y, z)].type);
	//}
	//chunk->gpu_data_used += 54;
	block_type b_type = chunk->blocks[to_1d_array(x, y, z)].type;

	//if (block_is_transparent(b_type))
	//{
	//	int i = 0;
	//	while (i != vert_count)
	//	{
	//		int result = 0;
	//		result = (data[i] + x) << 24;								//x
	//		result |= (data[i + 1] + y) << 16;							//y
	//		result |= (data[i + 2] + z) << 8;							//z
	//		result |= (data[i + 3]) << 7;								//u
	//		result |= (data[i + 4]) << 6;								//v
	//		result |= (block_get_texture(direction, b_type));			//b

	//		chunk->gpu_data_arr_transparent[chunk->blocks_in_use_transparent] = result;

	//		chunk->blocks_in_use_transparent += ATTRIBUTES_PER_VERTEX;
	//		i += 5;
	//	}
	//}
	//else
	{
		int i = 0;
		while (i != vert_count)
		{
			int result = 0;
			result = (data[i] + x) << 24;								//x
			result |= (data[i + 1] + y) << 16;							//y
			result |= (data[i + 2] + z) << 8;							//z
			result |= (data[i + 3]) << 7;								//u
			result |= (data[i + 4]) << 6;								//v
			auto b = block_get_texture(direction, b_type);
			result |= (b);			//b
			if (block_get_texture(direction, b_type) == 6)
			{
				int i = 0;
			}

			chunk->gpu_data_arr[chunk->blocks_in_use] = result;

			chunk->blocks_in_use += ATTRIBUTES_PER_VERTEX;
			i += 5;
		}
	}
}

void generate_face(chunk* current_chunk, const chunk* neighbor, const block_size_t data[30], block_face_direction direction, int x, int y, int z, int other_chunk_index, int current_chunk_index, bool on_edge)
{
	if (on_edge)
	{
		//Check if the block(that belongs to another chunk) is occupied
		//if (neighbor != nullptr && neighbor->blocks[other_chunk_index].type == block_type::AIR)
		if (neighbor != nullptr && block_is_transparent(neighbor->blocks[other_chunk_index].type))
		{
			add_face_and_texture(current_chunk, data, direction, x, y, z);
		}
	}
	// Is the block next to me occupied?
	//else if (current_chunk->blocks[current_chunk_index].type == block_type::AIR)
	else if (block_is_transparent(current_chunk->blocks[current_chunk_index].type))
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

#include "memory_arena.h"
void ChunkPrivate::generate_mesh(chunk* chunk, const glm::vec2& chunk_pos)
{
	return generate_mesh_timed(chunk, chunk_pos);
}

void ChunkPrivate::generate_mesh_timed(chunk* chunk, const glm::vec2& chunk_pos)
{
	using namespace std::chrono;
	using namespace ChunkPrivate;

	int triangle_count = 0;
	const int alloc_size = 1024 * 1024 * 100;

	auto c_in = steady_clock::now();
	if (chunk->back_neighbor == nullptr && chunk->chunks->contains(glm::ivec2((int)chunk_pos.x, (int)chunk_pos.y - CHUNK_SIZE_WIDTH)))
	{
		chunk->back_neighbor = &chunk->chunks->at(glm::ivec2((int)chunk_pos.x, (int)chunk_pos.y - CHUNK_SIZE_WIDTH));
	}
	if (chunk->front_neighbor == nullptr && chunk->chunks->contains(glm::ivec2((int)chunk_pos.x, (int)chunk_pos.y + CHUNK_SIZE_WIDTH)))
	{
		chunk->front_neighbor = &chunk->chunks->at(glm::ivec2((int)chunk_pos.x, (int)chunk_pos.y + CHUNK_SIZE_WIDTH));
	}
	if (chunk->left_neighbor == nullptr && chunk->chunks->contains(glm::ivec2((int)chunk_pos.x - CHUNK_SIZE_WIDTH, (int)chunk_pos.y)))
	{
		chunk->left_neighbor = &chunk->chunks->at(glm::ivec2((int)chunk_pos.x - CHUNK_SIZE_WIDTH, (int)chunk_pos.y));
	}
	if (chunk->right_neighbor == nullptr && chunk->chunks->contains(glm::ivec2((int)chunk_pos.x + CHUNK_SIZE_WIDTH, (int)chunk_pos.y)))
	{
		chunk->right_neighbor = &chunk->chunks->at(glm::ivec2((int)chunk_pos.x + CHUNK_SIZE_WIDTH, (int)chunk_pos.y));
	}
	auto c_ins = steady_clock::now();

	auto c_m = steady_clock::now();
	long long esc = 0;
	for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
	{
		for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
		{
			for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
			{
				int current = to_1d_array(x, y, z);
				if (chunk->blocks[current].type == block_type::AIR)
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
	auto c_ms = steady_clock::now();

	std::cout << "init_neighbors: " << duration_cast<milliseconds>(c_ins - c_in).count() << "ms\n";
	std::cout << "meshing: " << duration_cast<milliseconds>(c_ms - c_m).count() << "ms\n";
}
//void ChunkPrivate::generate_mesh(chunk* chunk, const glm::vec2& chunk_pos)
//{
//	using namespace ChunkPrivate;
//
//	int triangle_count = 0;
//	const int alloc_size = 1024 * 1024 * 100;
//
//	if (chunk->back_neighbor == nullptr && chunk->chunks->contains(glm::ivec2((int)chunk_pos.x, (int)chunk_pos.y - CHUNK_SIZE_WIDTH)))
//	{
//		chunk->back_neighbor = &chunk->chunks->at(glm::ivec2((int)chunk_pos.x, (int)chunk_pos.y - CHUNK_SIZE_WIDTH));
//	}
//	if (chunk->front_neighbor == nullptr && chunk->chunks->contains(glm::ivec2((int)chunk_pos.x, (int)chunk_pos.y + CHUNK_SIZE_WIDTH)))
//	{
//		chunk->front_neighbor = &chunk->chunks->at(glm::ivec2((int)chunk_pos.x, (int)chunk_pos.y + CHUNK_SIZE_WIDTH));
//	}
//	if (chunk->left_neighbor == nullptr && chunk->chunks->contains(glm::ivec2((int)chunk_pos.x - CHUNK_SIZE_WIDTH, (int)chunk_pos.y)))
//	{
//		chunk->left_neighbor = &chunk->chunks->at(glm::ivec2((int)chunk_pos.x - CHUNK_SIZE_WIDTH, (int)chunk_pos.y));
//	}
//	if (chunk->right_neighbor == nullptr && chunk->chunks->contains(glm::ivec2((int)chunk_pos.x + CHUNK_SIZE_WIDTH, (int)chunk_pos.y)))
//	{
//		chunk->right_neighbor = &chunk->chunks->at(glm::ivec2((int)chunk_pos.x + CHUNK_SIZE_WIDTH, (int)chunk_pos.y));
//	}
//
//	//chunk->gpu_data_length = extend(&chunk->gpu_data_arr, chunk->gpu_data_used, alloc_size);
//
//	for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
//	{
//		for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
//		{
//			for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
//			{
//				int current = to_1d_array(x, y, z);
//				if (chunk->blocks[current].type == block_type::AIR || chunk->blocks[current].type == block_type::WATER)
//					continue;
//
//				//add_face_and_texture(chunk, m_back_verticies, block_face_direction::BACK, x, y, z);
//				//add_face_and_texture(chunk, m_front_verticies, block_face_direction::FRONT, x, y, z);
//				//add_face_and_texture(chunk, m_left_verticies, block_face_direction::LEFT, x, y, z);
//				//add_face_and_texture(chunk, m_right_verticies, block_face_direction::RIGHT, x, y, z);
//				//add_face_and_texture(chunk, m_bottom_verticies, block_face_direction::BOTTOM, x, y, z);
//				//add_face_and_texture(chunk, m_top_verticies, block_face_direction::TOP, x, y, z);
//				generate_face(chunk, chunk->back_neighbor, m_back_verticies, block_face_direction::BACK, x, y, z, to_1d_array(x, y, CHUNK_SIZE_WIDTH - 1), to_1d_array(x, y, z - 1), z == 0);
//				generate_face(chunk, chunk->front_neighbor, m_front_verticies, block_face_direction::FRONT, x, y, z, to_1d_array(x, y, 0), to_1d_array(x, y, z + 1), (z + 1) >= CHUNK_SIZE_WIDTH);
//
//				generate_face(chunk, chunk->left_neighbor, m_left_verticies, block_face_direction::LEFT, x, y, z, to_1d_array(CHUNK_SIZE_WIDTH - 1, y, z), to_1d_array(x - 1, y, z), x == 0);
//				generate_face(chunk, chunk->right_neighbor, m_right_verticies, block_face_direction::RIGHT, x, y, z, to_1d_array(0, y, z), to_1d_array(x + 1, y, z), ((x + 1) >= CHUNK_SIZE_WIDTH));
//
//				// no chunk neighbors on the Y-axis
//				//if (y != 0 && (y == 0 || chunk->blocks[to_1d_array(x, y - 1, z)].type == block_type::AIR))
//				if (y != 0 && (y == 0 || block_is_transparent(chunk->blocks[to_1d_array(x, y - 1, z)].type)))
//				{
//					add_face_and_texture(chunk, m_bottom_verticies, block_face_direction::BOTTOM, x, y, z);
//				}
//
//				if (y + 1 > CHUNK_SIZE_HEIGHT || block_is_transparent(chunk->blocks[to_1d_array(x, y + 1, z)].type))
//				{
//					add_face_and_texture(chunk, m_top_verticies, block_face_direction::TOP, x, y, z);
//				}
//
//
//				//if (chunk->gpu_data_used > chunk->gpu_data_length - 2048)
//					//chunk->gpu_data_length = extend(&chunk->gpu_data_arr, chunk->gpu_data_used, alloc_size);
//			}
//		}
//	}
//}

void ChunkPrivate::calculate_lighting(chunk* chunk, const glm::vec2& chunk_pos)
{
	//for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
	//{
	//	for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
	//	{
	//		for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
	//		{
	//			int current = to_1d_array(x, y, z);
	//		}
	//	}
	//}
}

void ChunkPrivate::draw(const chunk& chunk)
{
	glBindVertexArray(chunk.vao_handle);
	glDrawArrays(GL_TRIANGLES, 0, chunk.blocks_in_use);
}