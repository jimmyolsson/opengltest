#pragma once
#include <glm/glm.hpp>
#include "robin_hood.h"

#include <vector>
#include "blocks/block.h"
#include "graphics/renderer.h"

enum BlockType : int;
enum class BlockFaceDirection : int;

struct Chunk;
typedef robin_hood::unordered_flat_map<glm::ivec2, Chunk> chunk_map_t;

static const int CHUNK_SIZE_WIDTH = 32;
static const int CHUNK_SIZE_HEIGHT = 255;
static const int CHUNK_DRAW_DISTANCE = 2;
static const int TOTAL_CHUNKS = CHUNK_DRAW_DISTANCE * CHUNK_DRAW_DISTANCE;
static const int BLOCKS_IN_CHUNK = CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT * CHUNK_SIZE_WIDTH;

struct GPUData
{
	float x, y, z;
	// Bitfield
	unsigned int info;
	// Bitfield
	unsigned int lighting_levels;
};

struct Chunk
{
	chunk_map_t* chunks;
	glm::ivec2 world_pos;
	Block* blocks;

	Chunk* front_neighbor = nullptr;
	Chunk* back_neighbor = nullptr;
	Chunk* right_neighbor = nullptr;
	Chunk* left_neighbor = nullptr;

	std::vector<GPUData> gpu_data_opaque = {};
	int verts_in_use = 0;

	std::vector<GPUData> gpu_data_transparent = {};
	int verts_in_use_transparent = 0;

	std::vector<GPUData> gpu_data_veg = {};
	int verts_in_use_veg = 0;

	unsigned int vao_handle_opaque = -1;
	unsigned int vbo_handle_opaque = -1;

	unsigned int vao_handle_transparent = -1;
	unsigned int vbo_handle_transparent = -1;

	unsigned int vao_handle_veg = -1;
	unsigned int vbo_handle_veg = -1;

	bool initialized = false;
	bool needs_remesh = false;
	bool needs_light_recalc = false;
	bool needs_light_reset = false;
};

// We need to keep track of what lights are in the world so that we can calculate them first  
// If we remove a block in chunk x how do we know that its not affected by lights in chunk y?
// Recalc the whole visible world?
struct Light
{
	glm::ivec2 chunk_index;
	int block_index;
};
static std::vector<Light> LightsInWorld;

void chunk_set_block(Chunk* c, glm::ivec3 block_pos, BlockType new_type);

// If the coordinates are outside of the bounds of the chunk, it will return the neighboring chunk
struct GetBlockResult
{
	Block* b;
	int block_index;
	Chunk* c;
};

GetBlockResult chunk_get_block(Chunk* c, glm::ivec3 block_pos);
GetBlockResult chunk_get_block(Chunk* c, short x, short y, short z);
std::vector<Block*> get_blocks_in_circle(Chunk* chunk, glm::ivec3 center, int radius);
void set_chunk_neighbors(Chunk* chunk);

// Exposed just so that we can multithread init
void chunk_generate_mesh(Chunk* chunk);
void chunk_generate_buffers(Chunk* self);

void chunk_update(chunk_map_t* chunks, glm::vec3 camera_pos);
void chunk_render_opaque(Chunk* chunk, Renderer* renderer, glm::mat4 view, glm::vec3 position);
void chunk_render_transparent(Chunk* chunk, Renderer* renderer, glm::mat4 view, glm::vec3 position);
void chunk_render_veg(Chunk* chunk, Renderer* renderer, glm::mat4 view, glm::vec3 position);
