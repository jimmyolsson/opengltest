#pragma once
#include "irrKlang.h"
#include "blocks/block.h"

struct sound_manager_s
{
	irrklang::ISoundEngine* sound_engine;
};

void sound_init(sound_manager_s* s);
void sound_play_block_sound(sound_manager_s* s, BlockType type, bool remove);
