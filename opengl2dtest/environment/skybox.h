#pragma once

#include "../graphics/renderer.h"

struct Skybox
{
	Cube cube;
};

Skybox skybox_create()
{
	Skybox skybox;
	skybox.cube = cube_create(TEXTURE_CUBEMAP_SKYBOX,
		SHADER_CUBEMAP,
		glm::vec3(0),
		glm::vec3(1));

	return skybox;
}

void skybox_render(Skybox* self, Renderer* renderer, glm::mat4 vieww)
{
	const auto view = glm::mat4(glm::mat3(vieww)); // remove translation from the view matrix
	renderer_render_cube(renderer, view, &self->cube);
}
