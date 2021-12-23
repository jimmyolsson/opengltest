#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>
#include "noise.h"

#include <iostream>
#include <string>

struct block
{
	int position;
};

class block_vertex_builder
{
public:
	//block_vertex_builder(int* block_3darray, int begin, int end)
	block_vertex_builder(int size)
		: m_chunk_size(size)
	{
		m_cubes = new char[pow(m_chunk_size, 3)];

		init_cubes();

		generate_noise();

		build_mesh();

		glGenVertexArrays(1, &m_array_handle);
		glBindVertexArray(m_array_handle);

		glGenBuffers(1, &m_buffer_handle);
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer_handle);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, m_data.size() * sizeof(float), m_data.data(), GL_STATIC_DRAW);

		//for(int i = 0;i < m_data.size(); i += 5)
		//{
		//	for (int j = 0; j <= 4; j++)
		//	{
		//		if (std::to_string(m_data[i + j])[0] == '-')
		//			std::cout << m_data[i + j] << ",";
		//		else
		//			std::cout << ' ' << m_data[i + j] << ",";

		//		if (j == 4)
		//			std::cout << "\n";
		//	}
		//}

		// I ?? 3 * vertex size
		//glVertexAttribIPointer(0, 1, GL_INT, m_vertex_size, (void*)0);
		// Position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

		// Texture coord
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void generate_noise()
	{
		OSN::Noise<2> noise;
		int width = m_chunk_size;
		int height = m_chunk_size;
		constexpr float featureSize = 24.0f;

		std::vector<glm::vec3> cubePositions;
		for (int x = 0; x < width; x++)
		{
			double xi = x / featureSize;
			for (int z = 0; z < height; z++)
			{
				double zi = z / featureSize;
				double noiseEval = noise.eval<double>(xi, zi) * 50; // ???
				for (int y = 0; y < noiseEval; y++)
				{
					m_cubes[offset(x, y, z)] = 1;
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
					if (m_cubes[offset(x, y, z)] == 0)
						continue;
					//check neighbors
					//back
					if (z == 0 || m_cubes[offset(x, y, z - 1)] == 0)
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
						}
					}
					//front
					if (((z + 1) >= m_chunk_size) || m_cubes[offset(x, y, z + 1)] == 0)
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
						}
					}
					//left
					if (x == 0 || m_cubes[offset(x - 1, y, z)] == 0)
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
						}
					}
					//right
					if (((x + 1) >= m_chunk_size) || m_cubes[offset(x + 1, y, z)] == 0)
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
						}
					}
					//bottom
					if (y == 0 || m_cubes[offset(x, y - 1, z)] == 0)
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
						}
					}
					//top
					if (((y + 1) >= m_chunk_size) || m_cubes[offset(x, y + 1, z)] == 0)
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
						}
					}
				}
			}
		}
	}

	void draw()
	{
		glBindVertexArray(m_array_handle);
		glDrawArrays(GL_TRIANGLES, 0, m_data.size() / 5);
	}

private:
	int m_chunk_size;
	char* m_cubes;
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
			for (int y = 0; y < m_chunk_size; y++)
				for (int z = 0; z < m_chunk_size; z++)
					m_cubes[offset(x, y, z)] = 0;
	}

	float vertices[180] = {
		//front
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, 1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f
	};


	const float m_back_verticies[30] = {
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f
	};
	const float m_front_verticies[30] = {
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f
	};
	const float m_left_verticies[30] = {
		-0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, 1.0f, 0.0f
	};
	const float m_right_verticies[30] = {
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f
	};
	const float m_bottom_verticies[30] = {
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f
	};
	const float m_top_verticies[30] = {
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f
	};
};
