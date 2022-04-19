#pragma once
#include <glm/glm.hpp>
#include "robin_hood.h"

#include <vector>

struct block;
enum class block_type;
enum class block_face_direction;

const int CHUNK_SIZE_WIDTH = 32;
const int CHUNK_SIZE_HEIGHT = 32;
const int CHUNK_DRAW_DISTANCE = 40;
const int TOTAL_CHUNKS = CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE;

typedef short block_size_t;
const int BLOCK_SIZE_BYTES = sizeof(block_size_t);

inline static int to_1d_array(int x, int y, int z)
{
	return (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x;
}

struct chunk
{
	block* blocks;

	const robin_hood::unordered_flat_map<glm::ivec2, chunk>* chunks;

	block_size_t* gpu_data_arr = nullptr;
	int gpu_data_length = 0;
	int gpu_data_used = 0;
	int gpu_data_last_used = 0;

	const chunk* front_neighbor = nullptr;
	const chunk* back_neighbor = nullptr;
	const chunk* right_neighbor = nullptr;
	const chunk* left_neighbor = nullptr;

	unsigned int vao_handle = -1;
	unsigned int vbo_handle = -1;

	bool initialized = false;
};

namespace ChunkPrivate
{
	void update_buffers(chunk& chunk);
	void init_buffers(chunk& chunk);
	void generate_mesh(chunk& chunk, const glm::vec2& chunk_pos);
	void draw(chunk& chunk);

	const static block_size_t m_back_verticies[30] = {
		0, 0, 0, 0, 0, // Bottom-left
		1, 0, 0, 1, 0, // bottom-right    
		1, 1, 0, 1, 1, // top-right              
		1, 1, 0, 1, 1, // top-right
		0, 1, 0, 0, 1, // top-left
		0, 0, 0, 0, 0, // bottom-left    
	};
	const static block_size_t m_front_verticies[30] = {
		0, 0, 1, 0, 0, // bottom-left
		1, 1, 1, 1, 1, // top-right
		1, 0, 1, 1, 0, // bottom-right        
		1, 1, 1, 1, 1, // top-right
		0, 0, 1, 0, 0, // bottom-left
		0, 1, 1, 0, 1, // top-left     
	};
	const static block_size_t m_left_verticies[30] = {
		0, 1, 1, 1, 1, // top-right
		0, 0, 0, 0, 1, // bottom-left
		0, 1, 0, 0, 1, // top-left       
		0, 0, 0, 0, 0, // bottom-left
		0, 1, 1, 1, 1, // top-right
		0, 0, 1, 1, 0, // bottom-right

		//0,  1,  1,  1, 0, // top-right
		//0, 0, 0,  0, 1, // bottom-left
		//0,  1, 0,  1, 1, // top-left       
		//0, 0, 0,  0, 1, // bottom-left
		//0,  1,  1,  1, 0, // top-right
		//0, 0,  1,  0, 0, // bottom-right
	};
	const static block_size_t m_right_verticies[30] = {
		 1, 1, 1, 1, 0, // top-left
		 1, 1, 0, 1, 1, // top-right      
		 1, 0, 0, 0, 1, // bottom-right          
		 1, 0, 0, 0, 1, // bottom-right
		 1, 0, 1, 0, 0, // bottom-left
		 1, 1, 1, 1, 0, // top-left
	};
	const static block_size_t m_bottom_verticies[30] = {
		0, 0, 0, 0, 1, // top-right
		1, 0, 1, 1, 0, // bottom-left
		1, 0, 0, 1, 1, // top-left        
		1, 0, 1, 1, 0, // bottom-left
		0, 0, 0, 0, 1, // top-right
		0, 0, 1, 0, 0, // bottom-right
	};
	const static block_size_t m_top_verticies[30] = {
		0, 1, 0, 0, 1, // top-left
		1, 1, 0, 1, 1, // top-right
		1, 1, 1, 1, 0, // bottom-right                 
		1, 1, 1, 1, 0, // bottom-right
		0, 1, 1, 0, 0, // bottom-left  
		0, 1, 0, 0, 1  // top-left  
	};


	/*	const static float m_back_verticies[30] = {
		0, 0, 0,  0, 0, // Bottom-left
		 1, 0, 0,  1, 0, // bottom-right    
		 1,  1, 0,  1, 1, // top-right              
		 1,  1, 0,  1, 1, // top-right
		0,  1, 0,  0, 1, // top-left
		0, 0, 0,  0, 0, // bottom-left    
	};
	const static float m_front_verticies[30] = {
		0, 0,  1,  0, 0, // bottom-left
		 1,  1,  1,  1, 1, // top-right
		 1, 0,  1,  1, 0, // bottom-right        
		 1,  1,  1,  1, 1, // top-right
		0, 0,  1,  0, 0, // bottom-left
		0,  1,  1,  0, 1, // top-left     
	};
	const static float m_left_verticies[30] = {
		0,  1,  1,  1, 1, // top-right
		0, 0, 0,  0, 1, // bottom-left
		0,  1, 0,  0, 1, // top-left       
		0, 0, 0,  0, 0, // bottom-left
		0,  1,  1,  1, 1, // top-right
		0, 0,  1,  1, 0, // bottom-right

		//0,  1,  1,  1, 0, // top-right
		//0, 0, 0,  0, 1, // bottom-left
		//0,  1, 0,  1, 1, // top-left       
		//0, 0, 0,  0, 1, // bottom-left
		//0,  1,  1,  1, 0, // top-right
		//0, 0,  1,  0, 0, // bottom-right
	};
	const static float m_right_verticies[30] = {
		 1,  1,  1,  1, 0, // top-left
		 1,  1, 0,  1, 1, // top-right      
		 1, 0, 0,  0, 1, // bottom-right          
		 1, 0, 0,  0, 1, // bottom-right
		 1, 0,  1,  0, 0, // bottom-left
		 1,  1,  1,  1, 0, // top-left
	};
	const static float m_bottom_verticies[30] = {
		0, 0, 0,  0, 1, // top-right
		 1, 0,  1,  1, 0, // bottom-left
		 1, 0, 0,  1, 1, // top-left        
		 1, 0,  1,  1, 0, // bottom-left
		0, 0, 0,  0, 1, // top-right
		0, 0,  1,  0, 0, // bottom-right
	};
	const static float m_top_verticies[30] = {
		0,  1, 0,  0, 1, // top-left
		 1,  1, 0,  1, 1, // top-right
		 1,  1,  1,  1, 0, // bottom-right                 
		 1,  1,  1,  1, 0, // bottom-right
		0,  1,  1,  0, 0, // bottom-left  
		0,  1, 0,  0, 1  // top-left  
	};
*/

}
