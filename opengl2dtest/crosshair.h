#pragma once
#include "glad/glad.h"
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/norm.hpp>

typedef struct crosshair_s {
	unsigned vao;
	unsigned vbo;
	shader_program s;
}crosshair_t;

void crosshair_render(crosshair_t* c, float screen_w, float screen_h)
{
	glBindVertexArray(c->vao);

	shader_use(&c->s);
	shader_set_mat4(&c->s, "transform", glm::scale<float>(glm::mat4(1), glm::vec3(1.0f, (float)screen_w / (float)screen_h, 1.0f)));

	glDrawArrays(GL_LINES, 0, 12);
	glBindVertexArray(0);
}

crosshair_t crosshair_create()
{
	crosshair_t c;
	float gpu_data[12] = {
		0.01f,
		0.0f,
		0.0f,
		-0.01f,
		0.0f,
		0.0f,
		0.0f,
		0.01f,
		0.0f,
		0.0f,
		-0.01f,
		0.0f,
	};

	shader_program sp;
	shader_load(&sp, "..\\resources\\shaders\\crosshair_vert.glsl", GL_VERTEX_SHADER);
	shader_load(&sp, "..\\resources\\shaders\\crosshair_frag.glsl", GL_FRAGMENT_SHADER);
	shader_link(&sp);

	c.s = sp;

	glGenVertexArrays(1, &c.vao);
	glBindVertexArray(c.vao);

	glGenBuffers(1, &c.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, c.vbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), gpu_data, GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return c;
}

