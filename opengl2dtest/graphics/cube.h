#pragma once
#include "glm/glm.hpp"
#include "renderer_resources.h"

struct Cube
{
	unsigned int vao;
	unsigned int vbo;

	TextureType texture_type;
	ShaderType shader_type;

	glm::vec3 position;
	glm::vec3 scale;
};

Cube cube_create(TextureType texture_type, ShaderType shader_type, glm::vec3 pos, glm::vec3 scale);

// x y z u v
const static int cube_verts[] = {
	1, 1, 0, 1, 0,
	1, 0, 0, 1, 1,
	0, 1, 0, 0, 0,
	1, 0, 0, 1, 1,
	0, 0, 0, 0, 1,
	0, 1, 0, 0, 0,
	0, 1, 1, 1, 0,
	0, 0, 1, 1, 1,
	1, 1, 1, 0, 0,
	0, 0, 1, 1, 1,
	1, 0, 1, 0, 1,
	1, 1, 1, 0, 0,
	0, 1, 0, 1, 0,
	0, 0, 0, 1, 1,
	0, 1, 1, 0, 0,
	0, 0, 0, 1, 1,
	0, 0, 1, 0, 1,
	0, 1, 1, 0, 0,
	1, 1, 1, 1, 0,
	1, 0, 1, 1, 1,
	1, 1, 0, 0, 0,
	1, 0, 1, 1, 1,
	1, 0, 0, 0, 1,
	1, 1, 0, 0, 0,
	1, 0, 1, 1, 0,
	0, 0, 1, 1, 1,
	1, 0, 0, 0, 0,
	0, 0, 1, 1, 1,
	0, 0, 0, 0, 1,
	1, 0, 0, 0, 0,
	0, 1, 1, 1, 0,
	1, 1, 1, 1, 1,
	0, 1, 0, 0, 0,
	1, 1, 1, 1, 1,
	1, 1, 0, 0, 1,
	0, 1, 0, 0, 0
};
