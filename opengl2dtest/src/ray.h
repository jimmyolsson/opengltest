#pragma once
#include <glm/glm.hpp>
#include "chunk.h"

struct ray_hit_result
{
	glm::ivec3 block_pos;
	glm::ivec3 direction;
	Chunk* chunk_hit;
	glm::ivec2 chunk_world_pos;
	bool top_half;
};

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;

	Ray(glm::vec3 o, glm::vec3 d)
		: origin(o), direction(d)
	{
	}
};

ray_hit_result ray_fire(Ray* self, float max_distance, chunk_map_t* chunks);
