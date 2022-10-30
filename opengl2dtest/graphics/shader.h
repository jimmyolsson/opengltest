#pragma once
#include <glm/glm.hpp>

struct ShaderProgram
{
	unsigned int handle = 0;
	int size = -1;
	int shaders[3];
};

// TODO: Improve logging
ShaderProgram shader_create(const char* vs_path, const char* fs_path);
void shader_use(ShaderProgram* sp);
void shader_set_mat4(ShaderProgram* sp, const char* name, glm::mat4 value);
void shader_set_int(ShaderProgram* sp, const char* name, int value);