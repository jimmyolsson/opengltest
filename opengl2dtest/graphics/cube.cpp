#include "cube.h"
#include "glad/glad.h"

#include <vector>

void _create_buffers(unsigned int* vao, unsigned int* vbo)
{
	int index = 0;
	std::vector<int> gpu_data;
	for (int i = 0; i < 180-1; i++)
	{
		gpu_data.push_back(cube_verts[i]);
	}

	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, gpu_data.size() * sizeof(int), gpu_data.data(), GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 5 * sizeof(int), (void*)0);
	glEnableVertexAttribArray(0);

	// Texture coord
	glVertexAttribPointer(1, 2, GL_UNSIGNED_INT, GL_FALSE, 5 * sizeof(int), (void*)(3 * sizeof(int)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Cube cube_create(TextureType texture_type, ShaderType shader_type, glm::vec3 pos, glm::vec3 scale)
{
	Cube cube{};

	cube.texture_type = texture_type;
	cube.shader_type = shader_type;
	cube.position = pos;
	cube.scale = scale;

	_create_buffers(&cube.vao, &cube.vbo);

	return cube;
}
