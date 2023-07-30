#include "block.h"

#include <assert.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "blocks.h"
#include "../util/common.h"

void block_get_name(BlockType type, char* name, int buffer_size)
{
	switch (type)
	{
	case BlockType::STONE:
		strncpy(name, "STONE", buffer_size);
		return;
	case BlockType::DIRT:
		strncpy(name, "DIRT", buffer_size);
		return;
	case BlockType::DIRT_GRASS:
		strncpy(name, "DIRT_GRASS", buffer_size);
		return;
	case BlockType::SAND:
		strncpy(name, "SAND", buffer_size);
		return;
	case BlockType::LEAVES:
		strncpy(name, "LEAVES", buffer_size);
		return;
	case BlockType::OAK_LOG:
		strncpy(name, "OAK_LOG", buffer_size);
		return;
	case BlockType::WATER:
		strncpy(name, "WATER", buffer_size);
		return;
	case BlockType::GLASS_PANE:
		strncpy(name, "GLASS_PANE", buffer_size);
		return;	
	case BlockType::BRICKS:
		strncpy(name, "BRICKS", buffer_size);
		return;	
	case BlockType::CONCRETE_WHITE:
		strncpy(name, "CONCRETE_WHITE", buffer_size);
		return;
	case BlockType::GLASS:
		strncpy(name, "BRICKS", buffer_size);
		return;	
	case BlockType::GRASS:
		strncpy(name, "GRASS", buffer_size);
		return;	

	default:
		assert(false);
	}
	name[buffer_size - 1] = '\0';
}

int block_get_texture(BlockFaceDirection direction, BlockType type)
{
	switch (direction)
	{
	case BlockFaceDirection::BACK:
		return (int)block_infos[type].texture_index_back;
		break;
	case BlockFaceDirection::FRONT:
		return (int)block_infos[type].texture_index_front;
		break;
	case BlockFaceDirection::RIGHT:
		return (int)block_infos[type].texture_index_right;
		break;
	case BlockFaceDirection::LEFT:
		return (int)block_infos[type].texture_index_left;
		break;
	case BlockFaceDirection::BOTTOM:
		return (int)block_infos[type].texture_index_bottom;
		break;
	case BlockFaceDirection::TOP:
		return (int)block_infos[type].texture_index_top;
		break;
	default:
		assert(false);
	}
}

void block_get_sound(BlockType type, bool remove, char* name, int buffer_size)
{
	char n[50];
	BlockType sound_type = remove ? block_infos[type].sound_remove : block_infos[type].sound_place;
	block_get_name(sound_type, n, sizeof(n));

	// We have multiple sounds for some blocks
	int num = random(1, 4);

	snprintf(n, sizeof(n), "%s%d", n, num);

	name[buffer_size - 1] = '\0';
	strcpy(name, n);
}

bool block_is_transparent(BlockType type)
{
	return block_infos[type].is_transparent && type != BlockType::AIR;
}

