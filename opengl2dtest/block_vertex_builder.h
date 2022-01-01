#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <iostream>
#include <string>

#include "blocks/block.h"

class block_vertex_builder
{
public:
	block_vertex_builder(int size, int x_size, int y_size, int z_size)
		: m_chunk_size(size), x_size(x_size), y_size(y_size), z_size(z_size)
	{
	}
	void setup_buffers()
	{
		glGenVertexArrays(1, &m_array_handle);
		glBindVertexArray(m_array_handle);

		glGenBuffers(1, &m_buffer_handle);
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer_handle);
		glBufferData(GL_ARRAY_BUFFER, m_data.size() * sizeof(float), m_data.data(), GL_STATIC_DRAW);

		// Position
		auto a = sizeof(block);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

		// Texture coord
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

		// Blocktype
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void build_mesh()
	{
		for (int x = 0; x < x_size; x++)
		{
			for (int y = 0; y < y_size; y++)
			{
				for (int z = 0; z < z_size; z++)
				{
					int vert_count = 30;

					if (m_blocks[offset(x, y, z)].type == block_type::AIR)
						continue; 

					if (z == 0 || m_blocks[offset(x, y, z - 1)].type == block_type::AIR)
					{
						int i = 0;
						while (i != vert_count)
						{
							m_data.push_back(m_back_verticies[i] + x);
							i++;
							m_data.push_back(m_back_verticies[i] + y);
							i++;
							m_data.push_back(m_back_verticies[i] + z);
							i++;
							m_data.push_back(m_back_verticies[i]);
							i++;
							m_data.push_back(m_back_verticies[i]);
							i++;
							auto a = block_get_texture(block_face_direction::BACK, m_blocks[offset(x, y, z)].type);
							m_data.push_back(a);
						}
					}
					//front
					if (((z + 1) >= z_size) || m_blocks[offset(x, y, z + 1)].type == block_type::AIR)
					{
						int i = 0;
						while (i != vert_count)
						{
							m_data.push_back(m_front_verticies[i] + x);
							i++;
							m_data.push_back(m_front_verticies[i] + y);
							i++;
							m_data.push_back(m_front_verticies[i] + z);
							i++;
							m_data.push_back(m_front_verticies[i]);
							i++;
							m_data.push_back(m_front_verticies[i]);
							i++;
							auto a = block_get_texture(block_face_direction::FRONT, m_blocks[offset(x, y, z)].type);
							m_data.push_back(a);
						}
					}
					//left
					if (x == 0 || m_blocks[offset(x - 1, y, z)].type == block_type::AIR)
					{
						int i = 0;
						while (i != vert_count)
						{
							m_data.push_back(m_left_verticies[i] + x);
							i++;
							m_data.push_back(m_left_verticies[i] + y);
							i++;
							m_data.push_back(m_left_verticies[i] + z);
							i++;
							m_data.push_back(m_left_verticies[i]);
							i++;
							m_data.push_back(m_left_verticies[i]);
							i++;
							auto a = block_get_texture(block_face_direction::LEFT, m_blocks[offset(x, y, z)].type);
							m_data.push_back(a);
						}
					}
					//right
					if (((x + 1) >= x_size) || m_blocks[offset(x + 1, y, z)].type == block_type::AIR)
					{
						int i = 0;
						while (i != vert_count)
						{
							m_data.push_back(m_right_verticies[i] + x);
							i++;
							m_data.push_back(m_right_verticies[i] + y);
							i++;
							m_data.push_back(m_right_verticies[i] + z);
							i++;
							m_data.push_back(m_right_verticies[i]);
							i++;
							m_data.push_back(m_right_verticies[i]);
							i++;
							auto a = block_get_texture(block_face_direction::RIGHT, m_blocks[offset(x, y, z)].type);
							m_data.push_back(a);
						}

					}
					//bottom
					if (y == 0 || m_blocks[offset(x, y - 1, z)].type == block_type::AIR)
					{
						int i = 0;
						while (i != vert_count)
						{
							m_data.push_back(m_bottom_verticies[i] + x);
							i++;
							m_data.push_back(m_bottom_verticies[i] + y);
							i++;
							m_data.push_back(m_bottom_verticies[i] + z);
							i++;
							m_data.push_back(m_bottom_verticies[i]);
							i++;
							m_data.push_back(m_bottom_verticies[i]);
							i++;
							auto a = block_get_texture(block_face_direction::BOTTOM, m_blocks[offset(x, y, z)].type);
							m_data.push_back(a);
						}
					}
					// top
					if (((y + 1) >= y_size) || m_blocks[offset(x, y + 1, z)].type == block_type::AIR)
					{
						int i = 0;
						while (i != vert_count)
						{
							m_data.push_back(m_top_verticies[i] + x);
							i++;
							m_data.push_back(m_top_verticies[i] + y);
							i++;
							m_data.push_back(m_top_verticies[i] + z);
							i++;
							m_data.push_back(m_top_verticies[i]);
							i++;
							m_data.push_back(m_top_verticies[i]);
							i++;
							auto a = block_get_texture(block_face_direction::TOP, m_blocks[offset(x, y, z)].type);
							m_data.push_back(a);
						}
					}
				}
			}
		}
	}

	void draw()
	{
		glBindVertexArray(m_array_handle);
		glDrawArrays(GL_TRIANGLES, 0, m_data.size() / 6);
	}

	block* m_blocks;
	glm::ivec3 pos;
private:
	int x_size;
	int y_size;
	int z_size;
	int m_chunk_size;
	std::vector<float> m_data;
	GLuint m_array_handle;
	GLuint m_buffer_handle;
	int m_vertex_size;

	int offset(int x, int y, int z)
	{
		//auto index = (z * x_size * y_size) + (y * x_size) + x;
		return (x * x_size) + (y * y_size) + z;
		//return (x * x_sizease) + (y * y_size) + z;
	}

	const float m_back_verticies[30] = {
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
	};
	const float m_front_verticies[30] = {
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
	};
	const float m_left_verticies[30] = {
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	};
	const float m_right_verticies[30] = {
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
	};
	const float m_bottom_verticies[30] = {
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
	};
	const float m_top_verticies[30] = {
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
	};
};
