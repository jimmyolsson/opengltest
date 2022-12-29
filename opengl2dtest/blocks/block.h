#pragma once

#include <iostream>

const int TOTAL_ELEMENTS_IN_QUAD = 30;
typedef int block_size_t;
const int BLOCK_SIZE_BYTES = sizeof(block_size_t);

#define BLOCK_TYPE_LAST WATER
enum BlockType : int {
	AIR = 0,
	STONE,
	DIRT,
	DIRT_GRASS,
	SAND,
	LEAVES,
	OAK_LOG,
	WATER
};

enum class block_face_direction : int
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
	bool sky = false;
	BlockType type = BlockType::AIR;

	//char light_level_top = 0;
	//char light_level_bottom = 0;
	//char light_level_right = 0;
	//char light_level_left = 0;
	//char light_level_front = 0;
	//char light_level_back = 0;
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

int block_get_texture(block_face_direction direction, BlockType type);

void block_get_sound(BlockType type, bool remove, char* name, int buffer_size);

static bool block_is_translucent(BlockType type)
{
	switch (type)
	{
	case WATER:
		return true;
	default:
		return false;
	}
}

static bool block_is_transparent(BlockType type)
{
	switch (type)
	{
	case BlockType::AIR:
	case BlockType::LEAVES:
	case BlockType::WATER:
		return true;
		break;
	default:
		return false;
	}
}
