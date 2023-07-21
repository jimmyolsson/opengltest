#include "quad.h"
#include <stb_image.h>
#include "shader.h"
#include "../util/common_graphics.h"

#include <iostream>
#include <vector>

const int verts_uv[] = {
	1, 1, 0, 1, 0,
	1, 0, 0, 1, 1,
	0, 1, 0, 0, 0,
	1, 0, 0, 1, 1,
	0, 0, 0, 0, 1,
	0, 1, 0, 0, 0
};

const int verts[] = {
	1, 1, 0,
	1, 0, 0,
	0, 1, 0,
	1, 0, 0,
	0, 0, 0,
	0, 1, 0,
};
void _create_buffers(bool uses_texture, unsigned int* vao, unsigned int* vbo)
{
	int index = 0;
	std::vector<int> gpu_data;
	if (uses_texture)
	{
		for (int i = 0; i < 30; i++)
		{
			gpu_data.push_back(verts_uv[i]);
		}
	}
	else
	{
		for (int i = 0; i < 18; i++)
		{
			gpu_data.push_back(verts[i]);
		}
	}
	GL_CALL(glGenVertexArrays(1, vao));
	GL_CALL(glBindVertexArray(*vao));

	GL_CALL(glGenBuffers(1, vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, *vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, gpu_data.size() * sizeof(int), gpu_data.data(), GL_STATIC_DRAW));

	// Position
	GL_CALL(glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 5 * sizeof(int), (void*)0));
	GL_CALL(glEnableVertexAttribArray(0));

	if (uses_texture)
	{
		// Texture coord
		GL_CALL(glVertexAttribPointer(1, 2, GL_UNSIGNED_INT, GL_FALSE, 5 * sizeof(int), (void*)(3 * sizeof(int))));
		GL_CALL(glEnableVertexAttribArray(1));
	}

	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GL_CALL(glBindVertexArray(0));
}

Quad quad_create(TextureType type, glm::vec2 pos, glm::vec2 scale)
{
	Quad q{};

	q.texture_type = type;
	q.position = pos;
	q.scale = scale;

	_create_buffers(type != TextureType::TEXTURE_NONE, &q.vao, &q.vbo);

	return q;
}
