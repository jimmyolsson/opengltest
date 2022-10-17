#pragma once
#include "glm/glm.hpp"
#include "blocks/block.h"
#include "memory_arena.h"
#include <FastNoise/FastNoise.h>

void world_generate(block* blocks, memory_arena* pool, const int xoffset, const int zoffset, const int width, const int height);
