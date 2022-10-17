#include "block.h"

#include <assert.h>
#include "blocks.h"

void block_get_name(block_type type, char* name, int buffer_size)
{
	assert(buffer_size > 20);
	int buff_s = buffer_size - 1;
	switch (type)
	{
	case block_type::STONE:
		strncpy_s(name, buffer_size, "STONE", buff_s);
		return;
	case block_type::DIRT:
		strncpy_s(name, buffer_size, "DIRT", buff_s);
		return;
	case block_type::DIRT_GRASS:
		strncpy_s(name, buffer_size, "DIRT_GRASS", buff_s);
		return;
	case block_type::SAND:
		strncpy_s(name, buffer_size, "SAND", buff_s);
		return;
	case block_type::LEAVES:
		strncpy_s(name, buffer_size, "LEAVES", buff_s);
		return;
	case block_type::OAK_LOG:
		strncpy_s(name, buffer_size, "OAK_LOG", buff_s);
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

int block_get_texture(block_face_direction direction, block_type type)
{
	switch (direction)
	{
	case block_face_direction::BACK:
		return block_infos[type].texture_index_back;
		break;
	case block_face_direction::FRONT:
		return block_infos[type].texture_index_front;
		break;
	case block_face_direction::RIGHT:
		return block_infos[type].texture_index_right;
		break;
	case block_face_direction::LEFT:
		return block_infos[type].texture_index_left;
		break;
	case block_face_direction::BOTTOM:
		return block_infos[type].texture_index_bottom;
		break;
	case block_face_direction::TOP:
		return block_infos[type].texture_index_top;
		break;
	default:
		assert(false);
	}
}

void block_get_sound(block_type type, bool remove, char* name, int buffer_size)
{
	char n[50];
	block_type sound_type = remove ? block_infos[type].sound_remove : block_infos[type].sound_place;
	block_get_name(sound_type, n, sizeof(n));

	int num = random(1, 4);

	snprintf(n, sizeof(n), "%s%d", n, num);

	name[buffer_size - 1] = '\0';
	strcpy_s(name, buffer_size, n);
}
