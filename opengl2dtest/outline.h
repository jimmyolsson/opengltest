#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <glad/glad.h>

#include "common.h"
// should not be here
#include "chunk.h"
#include "shader.h"
#include "ray.h"

struct outline_block
{
	unsigned int vao;
	unsigned int vbo;
	glm::vec3 position;
	shader_program sp;
	bool visible = false;
};

outline_block outline_create()
{
	outline_block outline_b;

	shader_load(&outline_b.sp, "..\\resources\\shaders\\outline.shadervs", GL_VERTEX_SHADER);
	shader_load(&outline_b.sp, "..\\resources\\shaders\\outline.shaderfs", GL_FRAGMENT_SHADER);

	shader_link(&outline_b.sp);

	const int vert_count = 30;
	std::vector<int> gpu_data;
	{
		int index = 0;
		for (int i = 0; i < 6; i++)
		{
			gpu_data.push_back(m_back_verticies[index]);
			gpu_data.push_back(m_back_verticies[index + 1]);
			gpu_data.push_back(m_back_verticies[index + 2]);
			gpu_data.push_back(m_back_verticies[index + 3]);
			gpu_data.push_back(m_back_verticies[index + 4]);
			index += 5;
		}
	}
	{
		int index = 0;
		for (int i = 0; i < 6; i++)
		{
			gpu_data.push_back(m_front_verticies[index]);
			gpu_data.push_back(m_front_verticies[index + 1]);
			gpu_data.push_back(m_front_verticies[index + 2]);
			gpu_data.push_back(m_front_verticies[index + 3]);
			gpu_data.push_back(m_front_verticies[index + 4]);
			index += 5;
		}
	}
	{
		int index = 0;
		for (int i = 0; i < 6; i++)
		{
			gpu_data.push_back(m_left_verticies[index]);
			gpu_data.push_back(m_left_verticies[index + 1]);
			gpu_data.push_back(m_left_verticies[index + 2]);
			gpu_data.push_back(m_left_verticies[index + 3]);
			gpu_data.push_back(m_left_verticies[index + 4]);
			index += 5;
		}
	}
	{
		int index = 0;
		for (int i = 0; i < 6; i++)
		{
			gpu_data.push_back(m_right_verticies[index]);
			gpu_data.push_back(m_right_verticies[index + 1]);
			gpu_data.push_back(m_right_verticies[index + 2]);
			gpu_data.push_back(m_right_verticies[index + 3]);
			gpu_data.push_back(m_right_verticies[index + 4]);
			index += 5;
		}
	}
	{
		int index = 0;
		for (int i = 0; i < 6; i++)
		{
			gpu_data.push_back(m_bottom_verticies[index]);
			gpu_data.push_back(m_bottom_verticies[index + 1]);
			gpu_data.push_back(m_bottom_verticies[index + 2]);
			gpu_data.push_back(m_bottom_verticies[index + 3]);
			gpu_data.push_back(m_bottom_verticies[index + 4]);
			index += 5;
		}
	}
	{
		int index = 0;
		for (int i = 0; i < 6; i++)
		{
			gpu_data.push_back(m_top_verticies[index]);
			gpu_data.push_back(m_top_verticies[index + 1]);
			gpu_data.push_back(m_top_verticies[index + 2]);
			gpu_data.push_back(m_top_verticies[index + 3]);
			gpu_data.push_back(m_top_verticies[index + 4]);
			index += 5;
		}
	}

	glGenVertexArrays(1, &outline_b.vao);
	glBindVertexArray(outline_b.vao);

	glGenBuffers(1, &outline_b.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, outline_b.vbo);
	glBufferData(GL_ARRAY_BUFFER, gpu_data.size() * sizeof(int), gpu_data.data(), GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 5 * BLOCK_SIZE_BYTES, (void*)0);
	glEnableVertexAttribArray(0);

	// Texture coord
	glVertexAttribPointer(1, 2, GL_UNSIGNED_INT, GL_FALSE, 5 * BLOCK_SIZE_BYTES, (void*)(3 * BLOCK_SIZE_BYTES));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return outline_b;
}
#include <iostream>

void outline_render(outline_block* outline, glm::mat4 p, glm::mat4 v)
{
	if (!outline->visible)
		return;

	shader_use(&outline->sp);
	shader_set_mat4(&outline->sp, "projection", p);
	shader_set_mat4(&outline->sp, "view", v);
	shader_set_mat4(&outline->sp, "model", glm::translate(glm::mat4(1.0f), outline->position));

	glBindVertexArray(outline->vao);

	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void outline_update(outline_block* outline, glm::vec3 origin, glm::vec3 direction, chunk_map_t* chunks)
{
	Ray r(origin, direction);
	ray_hit_result result = r.intersect_block(20, chunks);

	outline->position = glm::vec3(result.chunk_world_pos.x + result.block_pos.x,
		result.block_pos.y,
		result.chunk_world_pos.y + result.block_pos.z);

	outline->visible = result.chunk_hit != nullptr;
}
