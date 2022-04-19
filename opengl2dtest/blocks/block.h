#pragma once

#include <iostream>


enum class block_type : int {
	AIR = 0,
	STONE = 1,
	DIRT = 2,
	DIRT_GRASS = 3,
	SAND = 4,
	WATER = 5,
	LEAVES = 6
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
};

static int block_get_texture_stone(block_face_direction direction)
{
	return 0;
}

static int block_get_texture_dirt(block_face_direction direction)
{
	return 1;
}

static int block_get_texture_grass(block_face_direction direction)
{
	switch (direction)
	{
	case block_face_direction::BACK:
	case block_face_direction::FRONT:
	case block_face_direction::RIGHT:
	case block_face_direction::LEFT:
		return 2;
		break;
	case block_face_direction::TOP:
		return 3;
		break;
	case block_face_direction::BOTTOM:
		return 1;
	}

}

static int block_get_texture_sand(block_face_direction direction)
{
	return 4;
}

static int block_get_texture_water(block_face_direction direction)
{
	return 5;
}

static int block_get_texture_leaves(block_face_direction direction)
{
	return 6;
}

static int block_get_texture(block_face_direction direction, block_type type)
{
	switch (type)
	{
	case block_type::STONE:
		return block_get_texture_stone(direction);
		break;
	case block_type::DIRT:
		return block_get_texture_dirt(direction);
		break;
	case block_type::DIRT_GRASS:
		return block_get_texture_grass(direction);
		break;
	case block_type::SAND:
		return block_get_texture_sand(direction);
		break;
	case block_type::WATER:
		return block_get_texture_water(direction);
		break;
	case block_type::LEAVES:
		return block_get_texture_leaves(direction);
		break;
	}
}
