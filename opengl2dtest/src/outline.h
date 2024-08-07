#pragma once

#include "util/common_graphics.h"

#include "graphics/renderer.h"
#include "chunk.h"

#include "ray.h"


#include <glm/glm.hpp>
#include <vector>

struct OutlineBlock
{
	bool visible = false;
	Cube cube;
};

OutlineBlock outline_create()
{
	OutlineBlock outline;
	outline.cube = cube_create(TEXTURE_NONE,
		SHADER_OUTLINE,
		glm::vec3(0),
		glm::vec3(1));

	return outline;
}

void outline_render(OutlineBlock* self, Renderer* renderer, glm::mat4 view)
{
	if (!self->visible)
		return;

	renderer_render_cube(renderer, view, &self->cube);
}

void outline_update(OutlineBlock* self, glm::vec3 origin, glm::vec3 direction, chunk_map_t* chunks)
{
	Ray r(origin, direction);
	ray_hit_result result = ray_fire(&r, 20, chunks);

	if (result.chunk_hit != nullptr)
	{
		self->cube.position = glm::vec3(result.chunk_world_pos.x + result.block_pos.x,
			result.block_pos.y,
			result.chunk_world_pos.y + result.block_pos.z);
		self->visible = true;

	}
	else
		self->visible = false;
}
