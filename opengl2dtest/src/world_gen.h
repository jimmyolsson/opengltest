#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "blocks/block.h"

struct structure_node
{
	glm::ivec3 pos;
	BlockType type;
};

void world_generate(block* blocks, float* noise, const int xoffset, const int zoffset, const int width, const int height);
std::vector<structure_node> sphere_algo(int x0, int y0, int z0, int r);
