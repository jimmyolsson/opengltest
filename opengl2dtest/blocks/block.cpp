#include "block.h"

#include <assert.h>
#include "blocks.h"

void block_get_name(BlockType type, char* name, int buffer_size)
{
	assert(buffer_size > 20);
	int buff_s = buffer_size - 1;
	switch (type)
	{
	case BlockType::STONE:
		strncpy_s(name, buffer_size, "STONE", buff_s);
		return;
	case BlockType::DIRT:
		strncpy_s(name, buffer_size, "DIRT", buff_s);
		return;
	case BlockType::DIRT_GRASS:
		strncpy_s(name, buffer_size, "DIRT_GRASS", buff_s);
		return;
	case BlockType::SAND:
		strncpy_s(name, buffer_size, "SAND", buff_s);
		return;
	case BlockType::LEAVES:
		strncpy_s(name, buffer_size, "LEAVES", buff_s);
		return;
	case BlockType::OAK_LOG:
		strncpy_s(name, buffer_size, "OAK_LOG", buff_s);
		return;
	case BlockType::WATER:
		strncpy_s(name, buffer_size, "WATER", buff_s);
		return;

	default:
		assert(false);
	}
	name[buff_s] = '\0';
}

int random(int min, int max)
{
	return (rand() % (max - min + 1)) + min;
}

int block_get_texture(BlockFaceDirection direction, BlockType type)
{
	switch (direction)
	{
	case BlockFaceDirection::BACK:
		return block_infos[type].texture_index_back;
		break;
	case BlockFaceDirection::FRONT:
		return block_infos[type].texture_index_front;
		break;
	case BlockFaceDirection::RIGHT:
		return block_infos[type].texture_index_right;
		break;
	case BlockFaceDirection::LEFT:
		return block_infos[type].texture_index_left;
		break;
	case BlockFaceDirection::BOTTOM:
		return block_infos[type].texture_index_bottom;
		break;
	case BlockFaceDirection::TOP:
		return block_infos[type].texture_index_top;
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

	int num = random(1, 4);

	snprintf(n, sizeof(n), "%s%d", n, num);

	name[buffer_size - 1] = '\0';
	strcpy_s(name, buffer_size, n);
}
