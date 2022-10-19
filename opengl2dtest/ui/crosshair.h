#pragma once
#include "glad/glad.h"
#include "graphics/shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/norm.hpp>

typedef struct crosshair_s {
	unsigned vao;
	unsigned vbo;
	shader_program s;
	unsigned int handle;
}crosshair_t;

void crosshair_render(crosshair_t* c, float screen_w, float screen_h, glm::mat4 p, glm::mat4 v)
{
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, c->handle);
	glBindVertexArray(c->vao);

	shader_use(&c->s);
	glm::mat4 model = glm::mat4(1.0f);
	int size = 20;
	model = glm::translate(model, glm::vec3(screen_w / 2 - size / 2, screen_h / 2 - size / 2, 1));
	model = glm::scale(model, glm::vec3(size, size, 1));
	shader_set_mat4(&c->s, "model", model);
	shader_set_mat4(&c->s, "view", v);
	shader_set_mat4(&c->s, "projection", p);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glActiveTexture(0);
}

void quad_createee(unsigned int* vao, unsigned int* vbo)
{
	int index = 0;
	std::vector<int> gpu_data;
	for (int i = 0; i < 30 - 1; i++)
	{
		gpu_data.push_back(m_back_verticies[i]);
	}
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, gpu_data.size() * sizeof(int), gpu_data.data(), GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 5 * BLOCK_SIZE_BYTES, (void*)0);
	glEnableVertexAttribArray(0);

	// Texture coord
	glVertexAttribPointer(1, 2, GL_UNSIGNED_INT, GL_FALSE, 5 * BLOCK_SIZE_BYTES, (void*)(3 * BLOCK_SIZE_BYTES));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void load_textureeee(unsigned int* handle)
{
	glGenTextures(1, handle);
	glBindTexture(GL_TEXTURE_2D, *handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width;
	int height;
	int nrChannels;
	unsigned char* data = stbi_load("..\\resources\\textures\\gui\\crosshair.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

crosshair_t crosshair_create()
{
	crosshair_t c;
	quad_createee(&c.vao, &c.vbo);

	shader_program sp;
	//shader_load(&sp, "..\\resources\\shaders\\basic_color_vert.glsl", GL_VERTEX_SHADER);
	//shader_load(&sp, "..\\resources\\shaders\\basic_color_frag.glsl", GL_FRAGMENT_SHADER);
	shader_load(&sp, "..\\resources\\shaders\\basic_texture_vert.glsl", GL_VERTEX_SHADER);
	shader_load(&sp, "..\\resources\\shaders\\basic_texture_frag.glsl", GL_FRAGMENT_SHADER);
	shader_link(&sp);

	load_textureeee(&c.handle);

	c.s = sp;

	return c;
}

