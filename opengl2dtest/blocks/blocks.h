#pragma once
#include "block.h"

struct block_info
{
	BlockType type;

	int texture_index_back;
	int texture_index_front;
	int texture_index_left;
	int texture_index_right;
	int texture_index_bottom;
	int texture_index_top;

	// so we can share sounds
	BlockType sound_place;
	BlockType sound_remove;

	/*
	* TODO:
	* Compress to bitmask
	*/
};

// DEFINED IN THE ORDER OF THE BlockType ENUM
static block_info block_infos[BLOCK_TYPE_LAST+1] =
{
	// AIR
	{
		BlockType::AIR,
		0, 0, 0, 0, 0, 0,
		BlockType::AIR,
		BlockType::AIR
	},
	// STONE
	{
		BlockType::STONE,
		0, 0, 0, 0, 0, 0,
		BlockType::STONE,
		BlockType::STONE
	},
	// DIRT
	{
		BlockType::DIRT,
		1, 1, 1, 1, 1, 1,
		BlockType::DIRT,
		BlockType::DIRT
	},
	// DIRT_GRASS
	{
		BlockType::DIRT_GRASS,
		2, 2, 2, 2, 1, 3,
		BlockType::DIRT_GRASS,
		BlockType::DIRT_GRASS
	},
	// SAND
	{
		BlockType::SAND,
		4, 4, 4, 4, 4, 4,
		BlockType::SAND,
		BlockType::SAND
	},
	// LEAVES
	{
		BlockType::LEAVES,
		5, 5, 5, 5, 5, 5,
		BlockType::DIRT_GRASS,
		BlockType::DIRT_GRASS
	},
	// OAK_LOG
	{
		BlockType::OAK_LOG,
		6, 6, 6, 6, 7, 7,
		BlockType::OAK_LOG,
		BlockType::OAK_LOG
	},
	// WATER
	{
		BlockType::WATER,
		8, 8, 8, 8, 8, 8,
		BlockType::WATER,
		BlockType::WATER
	},
	// GLASS PANE
	{
		BlockType::GLASS_PANE,
		10, 9, 9, 9, 9, 9,
		BlockType::GLASS_PANE,
		BlockType::GLASS_PANE
	},
	// BRICKS
	{
		BlockType::BRICKS,
		11, 11, 11, 11, 11, 11,
		BlockType::BRICKS,
		BlockType::BRICKS
	},
	// WHITE CONCRETE
	{
		BlockType::CONCRETE_WHITE,
		12, 12, 12, 12, 12, 12,
		BlockType::CONCRETE_WHITE,
		BlockType::CONCRETE_WHITE
	}
};
