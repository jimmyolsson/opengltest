#pragma once

#include <iostream>

static const int BLOCK_NUM_TYPES = 8;
const int TOTAL_ELEMENTS_IN_QUAD = 30;
typedef int block_size_t;
const int BLOCK_SIZE_BYTES = sizeof(block_size_t);

enum block_type : int {
	AIR = 0,
	STONE = 1,
	DIRT = 2,
	DIRT_GRASS = 3,
	SAND = 4,
	WATER = 5,
	LEAVES = 6,
	OAK_LOG = 7,
};

enum class block_face_direction : int
{
	BACK = 0,
	FRONT = 1,
	RIGHT = 2,
	LEFT = 3,
	BOTTOM = 4,
	TOP = 5
};

struct block
{
	block_type type = block_type::AIR;
	bool sky = false;

	char light_level_top = 0;
	char light_level_bottom = 0;
	char light_level_right = 0;
	char light_level_left = 0;
	char light_level_front = 0;
	char light_level_back = 0;
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

int block_get_texture(block_face_direction direction, block_type type);

void block_get_sound(block_type type, bool remove, char* name, int buffer_size);

static bool block_is_transparent(block_type type)
{
	switch (type)
	{
	case block_type::AIR:
	case block_type::WATER:
	case block_type::LEAVES:
		return true;
		break;
	default:
		return false;
	}
}
