#include "sound_manager.h"
#include "blocks/block.h"
#include "util/common.h"

using namespace irrklang;

void sound_init(sound_manager_s* s)
{
	s->sound_engine = irrklang::createIrrKlangDevice(irrklang::ESOD_AUTO_DETECT, irrklang::ESEO_LOAD_PLUGINS);
	s->sound_engine->setSoundVolume(0.4);
}

// Expand action, for now its only two
void sound_play_block_sound(sound_manager_s* s, BlockType type, bool remove)
{
#ifndef _DEBUG
	char block_name[50];
	block_get_sound(type, remove, block_name, sizeof(block_name));

	char file_name[50];
	snprintf(file_name, sizeof(file_name), "..\\resources\\sounds\\block_dig\\%s.oga", block_name);
	file_name[sizeof(file_name) - 1] = '\0';
	s->sound_engine->play2D(file_name, false);
#endif
}
