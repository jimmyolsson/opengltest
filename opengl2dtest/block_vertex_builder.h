#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "noise.h"

#include <iostream>
#include <string>

#include "blocks/block.h"

class block_vertex_builder
{
public:
	//block_vertex_builder(int* block_3darray, int begin, int end)
	block_vertex_builder(int size)
		: m_chunk_size(size)
	{
		m_blocks = new block[pow(m_chunk_size, 3)];

		init_cubes();

		generate_noise();

		build_mesh();

		glGenVertexArrays(1, &m_array_handle);
		glBindVertexArray(m_array_handle);

		glGenBuffers(1, &m_buffer_handle);
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer_handle);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		//for (int i = 0; i < m_data.size() * sizeof(float); i++)
		//{
		//	std::cout << m_data[i];
		//}
		glBufferData(GL_ARRAY_BUFFER, m_data.size() * sizeof(float), m_data.data(), GL_STATIC_DRAW);

		//for(int i = 0;i < m_data.size(); i += 6)
		//{
		//	for (int j = 0; j <= 5; j++)
		//	{
		//		if (std::to_string(m_data[i + j])[0] == '-')
		//			std::cout << m_data[i + j] << ",";
		//		else
		//			std::cout << ' ' << m_data[i + j] << ",";

		//		if (j == 5)
		//			std::cout << "\n";
		//	}
		//}

		// I ?? 3 * vertex size
		//glVertexAttribIPointer(0, 1, GL_INT, m_vertex_size, (void*)0);
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

	void generate_noise()
	{
		OSN::Noise<2> noise;
		int width = m_chunk_size;
		int height = m_chunk_size;
		constexpr float featureSize = 10.0f;

		for (double x = 0; x < width; x++)
		{
			double xi = x / featureSize;
			for (double z = 0; z < height; z++)
			{
				double zi = z / featureSize;
				double noiseEval = noise.eval<double>(xi, zi) * 10; // ???
				for (double y = 0; y <= noiseEval; y++)
				{
					if (y > noiseEval-2 || y == noiseEval)
					{
						m_blocks[offset(x, y, z)].type = block_type::DIRT_GRASS;
					}
					else
						m_blocks[offset(x, y, z)].type = block_type::STONE;
				}
			}
		}
	}

	void build_mesh()
	{
		for (int x = 0; x < m_chunk_size; x++)
		{
			for (int y = 0; y < m_chunk_size; y++)
			{
				for (int z = 0; z < m_chunk_size; z++)
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
					if (((z + 1) >= m_chunk_size) || m_blocks[offset(x, y, z + 1)].type == block_type::AIR)
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
					if (((x + 1) >= m_chunk_size) || m_blocks[offset(x + 1, y, z)].type == block_type::AIR)
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
					if (((y + 1) >= m_chunk_size) || m_blocks[offset(x, y + 1, z)].type == block_type::AIR)
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

private:
	int m_chunk_size;
	block* m_blocks;
	std::vector<float> m_data;
	GLuint m_array_handle;
	GLuint m_buffer_handle;
	int m_vertex_size;

	int offset(int x, int y, int z)
	{
		return (z * m_chunk_size * m_chunk_size) + (y * m_chunk_size) + x;
	}

	void init_cubes()
	{
		for (int x = 0; x < m_chunk_size; x++)
		{
			for (int y = 0; y < m_chunk_size; y++)
			{
				for (int z = 0; z < m_chunk_size; z++)
				{
					if(y == 0)
						m_blocks[offset(x, y, z)].type = block_type::DIRT_GRASS;
					else
						m_blocks[offset(x, y, z)].type = block_type::AIR;
				}
			}
		}
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
