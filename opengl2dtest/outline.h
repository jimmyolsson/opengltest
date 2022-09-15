#pragma once

#include <glm/glm.hpp>

struct shader;

struct outline_block {
	unsigned int vao;
	unsigned int vbo;
	glm::ivec3 position;
};

 outline_block outline_create()
