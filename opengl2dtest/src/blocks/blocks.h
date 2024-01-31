#pragma once
#include "block.h"

struct block_info
{
	BlockType type;

	BlockTextureIndex texture_index_back;
	BlockTextureIndex texture_index_front;
	BlockTextureIndex texture_index_left;
	BlockTextureIndex texture_index_right;
	BlockTextureIndex texture_index_bottom;
	BlockTextureIndex texture_index_top;

	// NOTE: Air is transparent
	bool is_transparent;
	bool is_translucent;

	bool ao_enabled;

	// so we can share sounds
	BlockType sound_place;
	BlockType sound_remove;
};

// DEFINED IN THE ORDER OF THE BlockType ENUM
static block_info block_infos[BLOCK_TYPE_LAST + 1] =
{
	// AIR
	{
		BlockType::AIR,
		// This isnt rendered so the texture index isnt used
		BlockTextureIndex::STONE, // BACK
		BlockTextureIndex::STONE, // FRONT
		BlockTextureIndex::STONE, // LEFT
		BlockTextureIndex::STONE, // RIGHT
		BlockTextureIndex::STONE, // BOTTOM
		BlockTextureIndex::STONE, // TOP

		true, // TRANSPARENT
		true, // TRANSLUCENT

		false, // AO_ENABLED

		BlockType::AIR,
		BlockType::AIR
	},
	// STONE
	{
		BlockType::STONE,
		BlockTextureIndex::STONE, // BACK
		BlockTextureIndex::STONE, // FRONT
		BlockTextureIndex::STONE, // LEFT
		BlockTextureIndex::STONE, // RIGHT
		BlockTextureIndex::STONE, // BOTTOM
		BlockTextureIndex::STONE, // TOP

		false, // TRANSPARENT
		false, // TRANSLUCENT

		true, // AO_ENABLED

		BlockType::STONE,
		BlockType::STONE
	},
	// DIRT
	{
		BlockType::DIRT,
		BlockTextureIndex::DIRT, // BACK
		BlockTextureIndex::DIRT, // FRONT
		BlockTextureIndex::DIRT, // LEFT
		BlockTextureIndex::DIRT, // RIGHT
		BlockTextureIndex::DIRT, // BOTTOM
		BlockTextureIndex::DIRT, // TOP

		false, // TRANSPARENT
		false, // TRANSLUCENT

		true, // AO_ENABLED

		BlockType::DIRT,
		BlockType::DIRT
	},
	// DIRT_GRASS
	{
		BlockType::DIRT_GRASS,
		BlockTextureIndex::DIRT_GRASS_SIDE,	// BACK
		BlockTextureIndex::DIRT_GRASS_SIDE,	// FRONT
		BlockTextureIndex::DIRT_GRASS_SIDE,	// LEFT
		BlockTextureIndex::DIRT_GRASS_SIDE,	// RIGHT
		BlockTextureIndex::DIRT,			// BOTTOM
		BlockTextureIndex::DIRT_GRASS_TOP,	// TOP

		false, // TRANSPARENT
		false, // TRANSLUCENT

		true, // AO_ENABLED

		BlockType::DIRT_GRASS,
		BlockType::DIRT_GRASS
	},
	// SAND
	{
		BlockType::SAND,
		BlockTextureIndex::SAND, // BACK
		BlockTextureIndex::SAND, // FRONT
		BlockTextureIndex::SAND, // LEFT
		BlockTextureIndex::SAND, // RIGHT
		BlockTextureIndex::SAND, // BOTTOM
		BlockTextureIndex::SAND, // TOP

		false, // TRANSPARENT
		false, // TRANSLUCENT

		true, // AO_ENABLED

		BlockType::SAND,
		BlockType::SAND
	},
	// LEAVES
	{
		BlockType::LEAVES,
		BlockTextureIndex::LEAVES, // BACK
		BlockTextureIndex::LEAVES, // FRONT
		BlockTextureIndex::LEAVES, // LEFT
		BlockTextureIndex::LEAVES, // RIGHT
		BlockTextureIndex::LEAVES, // BOTTOM
		BlockTextureIndex::LEAVES, // TOP

		true, // TRANSPARENT
		false, // TRANSLUCENT

		true, // AO_ENABLED

		BlockType::DIRT_GRASS,
		BlockType::DIRT_GRASS
	},
	// OAK_LOG
	{
		BlockType::OAK_LOG,
		BlockTextureIndex::OAK_LOG, 	// BACK
		BlockTextureIndex::OAK_LOG, 	// FRONT
		BlockTextureIndex::OAK_LOG, 	// LEFT
		BlockTextureIndex::OAK_LOG, 	// RIGHT
		BlockTextureIndex::OAK_LOG_TOP, // BOTTOM
		BlockTextureIndex::OAK_LOG_TOP, // TOP

		false, // TRANSPARENT
		false, // TRANSLUCENT

		true, // AO_ENABLED

		BlockType::OAK_LOG,
		BlockType::OAK_LOG
	},
	// WATER
	{
		BlockType::WATER,
		BlockTextureIndex::WATER, // BACK
		BlockTextureIndex::WATER, // FRONT
		BlockTextureIndex::WATER, // LEFT
		BlockTextureIndex::WATER, // RIGHT
		BlockTextureIndex::WATER, // BOTTOM
		BlockTextureIndex::WATER, // TOP

		true, // TRANSPARENT
		true, // TRANSLUCENT

		false, // AO_ENABLED

		BlockType::WATER,
		BlockType::WATER
	},
	// GLASS PANE
	{
		BlockType::GLASS_PANE,
		BlockTextureIndex::GLASS_PANE_TOP,	// BACK
		BlockTextureIndex::GLASS, 			// FRONT
		BlockTextureIndex::GLASS, 			// LEFT
		BlockTextureIndex::GLASS, 			// RIGHT
		BlockTextureIndex::GLASS, 			// BOTTOM
		BlockTextureIndex::GLASS, 			// TOP

		true, // TRANSPARENT
		false, // TRANSLUCENT

		true, // AO_ENABLED

		BlockType::GLASS_PANE,
		BlockType::GLASS_PANE
	},
	// BRICKS
	{
		BlockType::BRICKS,
		BlockTextureIndex::BRICKS, // BACK
		BlockTextureIndex::BRICKS, // FRONT
		BlockTextureIndex::BRICKS, // LEFT
		BlockTextureIndex::BRICKS, // RIGHT
		BlockTextureIndex::BRICKS, // BOTTOM
		BlockTextureIndex::BRICKS, // TOP

		false, // TRANSPARENT
		false, // TRANSLUCENT

		true, // AO_ENABLED

		BlockType::BRICKS,
		BlockType::BRICKS
	},
	// WHITE CONCRETE
	{
		BlockType::CONCRETE_WHITE,
		BlockTextureIndex::WHITE_CONCRETE, // BACK
		BlockTextureIndex::WHITE_CONCRETE, // FRONT
		BlockTextureIndex::WHITE_CONCRETE, // LEFT
		BlockTextureIndex::WHITE_CONCRETE, // RIGHT
		BlockTextureIndex::WHITE_CONCRETE, // BOTTOM
		BlockTextureIndex::WHITE_CONCRETE, // TOP

		false, // TRANSPARENT
		false, // TRANSLUCENT

		true, // AO_ENABLED

		BlockType::CONCRETE_WHITE,
		BlockType::CONCRETE_WHITE
	},
	// GLASS
	{
		BlockType::GLASS,
		BlockTextureIndex::GLASS, // BACK
		BlockTextureIndex::GLASS, // FRONT
		BlockTextureIndex::GLASS, // LEFT
		BlockTextureIndex::GLASS, // RIGHT
		BlockTextureIndex::GLASS, // BOTTOM
		BlockTextureIndex::GLASS, // TOP

		true, // TRANSPARENT
		false, // TRANSLUCENT

		false, // AO_ENABLED

		BlockType::GLASS,
		BlockType::GLASS
	},
	// GRASS
	{
		BlockType::GRASS,
		BlockTextureIndex::GRASS, // BACK
		BlockTextureIndex::GRASS, // FRONT
		BlockTextureIndex::GRASS, // LEFT
		BlockTextureIndex::GRASS, // RIGHT
		BlockTextureIndex::GRASS, // BOTTOM
		BlockTextureIndex::GRASS, // TOP

		true, // TRANSPARENT
		false, // TRANSLUCENT

		false, // AO_ENABLED

		BlockType::GRASS,
		BlockType::GRASS
	}
};
