#pragma once
#include <glm/glm.hpp>
#include <unordered_map>

struct ShaderProgram
{
	unsigned int handle = 0;
	int size = -1;
	int shaders[3];

	unsigned int* uniform_locations;
	int uniform_count;
};

// TODO: Improve logging
ShaderProgram shader_create(const char* vs_path, const char* fs_path);
void shader_use(ShaderProgram* sp);
void shader_set_mat4(ShaderProgram* sp, unsigned int location, glm::mat4 value);
void shader_set_int(ShaderProgram* sp, unsigned int location, int value);