#pragma once
#include "blocks/block.h"

struct block_info
{
	block_type type;

	int texture_index_back;
	int texture_index_front;
	int texture_index_left;
	int texture_index_right;
	int texture_index_bottom;
	int texture_index_top;

	// so we can share sounds
	block_type sound_place;
	block_type sound_remove;

	/*
	* TODO:
	* Compress to bitmask
	*/
};

// DEFINED IN THE ORDER OF THE block_type ENUM
static block_info block_infos[BLOCK_TYPE_LAST+1] =
{
	// AIR
	{
		block_type::AIR,
		0, 0, 0, 0, 0, 0,
		block_type::AIR,
		block_type::AIR
	},
	// STONE
	{
		block_type::STONE,
		0, 0, 0, 0, 0, 0,
		block_type::STONE,
		block_type::STONE
	},
	// DIRT
	{
		block_type::DIRT,
		1, 1, 1, 1, 1, 1,
		block_type::DIRT,
		block_type::DIRT
	},
	// DIRT_GRASS
	{
		block_type::DIRT_GRASS,
		2, 2, 2, 2, 1, 3,
		block_type::DIRT_GRASS,
		block_type::DIRT_GRASS
	},
	// SAND
	{
		block_type::SAND,
		4, 4, 4, 4, 4, 4,
		block_type::SAND,
		block_type::SAND
	},
	// LEAVES
	{
		block_type::LEAVES,
		5, 5, 5, 5, 5, 5,
		block_type::DIRT_GRASS,
		block_type::DIRT_GRASS
	},
	// OAK_LOG
	{
		block_type::OAK_LOG,
		7, 7, 7, 7, 8, 8,
		block_type::OAK_LOG,
		block_type::OAK_LOG
	},
};
