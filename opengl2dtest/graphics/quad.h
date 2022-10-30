#pragma once
#include "glm/glm.hpp"
#include "renderer_resources.h"

struct Quad
{
	unsigned int vao;
	unsigned int vbo;

	TextureType texture_type;

	glm::vec2 position;
	glm::vec2 scale;
};

Quad quad_create(TextureType type, glm::vec2 pos, glm::vec2 scale);
