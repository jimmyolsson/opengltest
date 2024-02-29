#pragma once

const int TOTAL_ELEMENTS_IN_QUAD = 30;
typedef float block_size_t;
const int BLOCK_SIZE_BYTES = sizeof(block_size_t);
const int BLOCK_NAME_MAX_SIZE = 50;

#define BLOCK_TYPE_LAST BlockType::GRASS
enum BlockType : int
{
	AIR = 0,
	STONE,
	DIRT,
	DIRT_GRASS,
	SAND,
	LEAVES,
	OAK_LOG,
	WATER,
	GLASS_PANE,
	BRICKS,
	CONCRETE_WHITE,
	GLASS,
	GRASS,
};

#define BLOCK_TEXTURE_INDEX_LAST (int)BlockTextureIndex::GRASS
enum class BlockTextureIndex : int
{
	STONE = 0,
	DIRT,
	DIRT_GRASS_SIDE,
	DIRT_GRASS_TOP,
	SAND,
	LEAVES,
	OAK_LOG,
	OAK_LOG_TOP,
	WATER,
	GLASS,
	GLASS_PANE_TOP,
	BRICKS,
	WHITE_CONCRETE,
	GRASS,
};

enum class BlockFaceDirection : int
{
	BACK = 0,
	FRONT,
	RIGHT,
	LEFT,
	BOTTOM,
	TOP
};

struct block
{
	BlockType type = BlockType::AIR;
};
bool block_is_transparent(BlockType type);

const static block_size_t grass_1[TOTAL_ELEMENTS_IN_QUAD] = {
	1, 1, 0, 1, 0,
	1, 0, 0, 1, 1,
	0, 1, 1, 0, 0,
	1, 0, 0, 1, 1,
	0, 0, 1, 0, 1,
	0, 1, 1, 0, 0
};

const static block_size_t grass_2[TOTAL_ELEMENTS_IN_QUAD] = {
	1, 1, 1, 1, 0,
	1, 0, 1, 1, 1,
	0, 1, 0, 0, 0,
	1, 0, 1, 1, 1,
	0, 0, 0, 0, 1,
	0, 1, 0, 0, 0
};

// x y z u v
const static block_size_t m_back_verticies[TOTAL_ELEMENTS_IN_QUAD] = {
	1, 1, 0, 1, 0,
	1, 0, 0, 1, 1,
	0, 1, 0, 0, 0,
	1, 0, 0, 1, 1,
	0, 0, 0, 0, 1,
	0, 1, 0, 0, 0
};
const static block_size_t m_front_verticies[TOTAL_ELEMENTS_IN_QUAD] = {
	0, 1, 1, 1, 0,
	0, 0, 1, 1, 1,
	1, 1, 1, 0, 0,
	0, 0, 1, 1, 1,
	1, 0, 1, 0, 1,
	1, 1, 1, 0, 0
};
const static block_size_t m_left_verticies[TOTAL_ELEMENTS_IN_QUAD] = {
	0, 1, 0, 1, 0,
	0, 0, 0, 1, 1,
	0, 1, 1, 0, 0,
	0, 0, 0, 1, 1,
	0, 0, 1, 0, 1,
	0, 1, 1, 0, 0
};
const static block_size_t m_right_verticies[TOTAL_ELEMENTS_IN_QUAD] = {
	1, 1, 1, 1, 0,
	1, 0, 1, 1, 1,
	1, 1, 0, 0, 0,
	1, 0, 1, 1, 1,
	1, 0, 0, 0, 1,
	1, 1, 0, 0, 0
};
const static block_size_t m_bottom_verticies[TOTAL_ELEMENTS_IN_QUAD] = {
	1, 0, 1, 1, 0,
	0, 0, 1, 1, 1,
	1, 0, 0, 0, 0,
	0, 0, 1, 1, 1,
	0, 0, 0, 0, 1,
	1, 0, 0, 0, 0
};
const static block_size_t m_top_verticies[TOTAL_ELEMENTS_IN_QUAD] = {
	0, 1, 1, 1, 0,
	1, 1, 1, 1, 1,
	0, 1, 0, 0, 0,
	1, 1, 1, 1, 1,
	1, 1, 0, 0, 1,
	0, 1, 0, 0, 0
};

BlockTextureIndex block_get_texture(BlockFaceDirection direction, BlockType type);

void block_get_sound(BlockType type, bool remove, char* name, int buffer_size);