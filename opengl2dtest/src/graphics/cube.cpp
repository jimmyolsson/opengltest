#include "cube.h"
#include "../util/common_graphics.h"

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

const static float cube_vert_nouv[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

void _create_buffersuv(unsigned int* vao, unsigned int* vbo)
{
    GL_CALL(glGenVertexArrays(1, vao));
    GL_CALL(glGenBuffers(1, vbo));
    GL_CALL(glBindVertexArray(*vao));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, *vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vert_nouv), &cube_vert_nouv, GL_STATIC_DRAW));
    GL_CALL(glEnableVertexAttribArray(0));
    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
}

void _create_buffers(unsigned int* vao, unsigned int* vbo)
{
	GL_CALL(glGenVertexArrays(1, vao));
	GL_CALL(glBindVertexArray(*vao));

	GL_CALL(glGenBuffers(1, vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, *vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(cube_verts), &cube_verts, GL_STATIC_DRAW));

	// Position
	GL_CALL(glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 5 * sizeof(int), (void*)0));
	GL_CALL(glEnableVertexAttribArray(0));

	// Texture coord
	GL_CALL(glVertexAttribPointer(1, 2, GL_UNSIGNED_INT, GL_FALSE, 5 * sizeof(int), (void*)(3 * sizeof(int))));
	GL_CALL(glEnableVertexAttribArray(1));

	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GL_CALL(glBindVertexArray(0));
}

Cube cube_create(TextureType texture_type, ShaderType shader_type, glm::vec3 pos, glm::vec3 scale)
{
	Cube cube{};

	cube.texture_type = texture_type;
	cube.shader_type = shader_type;
	cube.position = pos;
	cube.scale = scale;

	// This is ugly
	if (texture_type == TextureType::TEXTURE_CUBEMAP_SKYBOX)
		_create_buffersuv(&cube.vao, &cube.vbo);
	else
		_create_buffers(&cube.vao, &cube.vbo);

	return cube;
}
