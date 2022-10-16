#pragma once
#include <glm/glm.hpp>

#define SHADERS_LAST SHADER_OUTLINE
enum shader_type {
	SHADER_CHUNK,
	SHADER_CROSSHAIR,
	SHADER_OUTLINE
};

struct shader_program
{
	unsigned int handle = 0;
	int size = -1;
	int shaders[3];
};

// TODO: Improve logging
void shader_load(shader_program* sp, const char* path, unsigned int type);
void shader_link(shader_program* sp);
void shader_use(shader_program* sp);
void shader_set_mat4(shader_program* sp, const char* name, glm::mat4 value);
void shader_set_int(shader_program* sp, const char* name, int value);