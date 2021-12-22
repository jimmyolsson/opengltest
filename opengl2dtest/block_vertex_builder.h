#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>
#include "noise.h"

struct block
{
	int position;
};

class block_vertex_builder
{
public:
	//block_vertex_builder(int* block_3darray, int begin, int end)
	block_vertex_builder()
	{
		m_cubez = new short[32768];

		init_cubes();

		//generate_noise();

		build_mesh();

		glGenVertexArrays(1, &m_array_handle);
		glBindVertexArray(m_array_handle);

		glGenBuffers(1, &m_buffer_handle);
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer_handle);
		glBufferData(GL_ARRAY_BUFFER, m_data.size() * sizeof(float), m_data.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);

		// I ?? 3 * vertex size
		//glVertexAttribIPointer(0, 1, GL_INT, m_vertex_size, (void*)0);
		m_vertex_size = sizeof(float);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void generate_noise()
	{
		OSN::Noise<2> noise;
		int width = chunk_size;
		int height = chunk_size;
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
					m_cubez[offset(x, y, z)] = 1;
					m_cubes[x][y][z] = 1;
				}
			}
		}
	}

	void build_mesh()
	{
		for (int x = 0; x < chunk_size; x++)
		{
			for (int y = 0; y < chunk_size; y++)
			{
				for (int z = 0; z < chunk_size; z++)
				{
					int vert_count = 18;
					if (m_cubes[x][y][z] == 0)
						continue;
					//check neighbors
					//front
					if (((z + 1) > chunk_size) || m_cubes[x][y][z + 1] == 0)
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
						}
						used += 6;
					}
					//back
					if (z == 0 || m_cubes[x][y][z - 1] == 0)
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
						}
						used += 6;
					}
					//left
					if (x == 0 || m_cubes[x - 1][y][z] == 0)
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
						}
						used += 6;
					}
					//right
					if (((x + 1) > chunk_size) || m_cubes[x + 1][y][z] == 0)
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
						}
						used += 6;
					}
					//bottom
					if (y == 0 || m_cubes[x][y - 1][z] == 0)
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
						}
						used += 6;
					}
					//top
					if (((y + 1) > chunk_size) || m_cubes[x][y + 1][z] == 0)
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
						}
						used += 6;
					}
				}
			}
		}
	}

	void draw()
	{
		glBindVertexArray(m_array_handle);
		glDrawArrays(GL_TRIANGLES, 0, m_data.size() / 3);
	}

private:
	int chunk_size = 50;
	int used = 0;
	short* m_cubez;
	short m_cubes[50][50][50];
	std::vector<float> m_data;
	GLuint m_array_handle;
	GLuint m_buffer_handle;
	int m_vertex_size;

	int offset(int x, int y, int z)
	{
		return (z * 32 * 32) + (y * 32) + x;
	}

	void init_cubes()
	{
		for (int x = 0; x < chunk_size; x++)
			for (int y = 0; y < chunk_size; y++)
				for (int z = 0; z < chunk_size; z++)
					m_cubes[x][y][z] = 0;
	}

	float vertices[108] = {
		//front
		-0.5f, -0.5f, -0.5f, // 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f, // 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, // 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, // 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, // 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, // 0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f, // 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, // 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, // 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, // 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f, // 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, // 0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, // 1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, // 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, // 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, // 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, // 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, // 1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f, // 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, // 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, // 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, // 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f, // 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, // 1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f, // 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, // 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f, // 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, // 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f, // 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, // 0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f, // 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, // 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, // 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, // 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, // 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, // 0.0f, 1.0f
	};


	const float m_front_verticies[18] = {
		-0.5f, -0.5f, -0.5f, // 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f, // 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, // 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, // 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, // 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, // 0.0f, 0.0f
	};
	const float m_back_verticies[18] = {
		-0.5f, -0.5f,  0.5f,  //0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  //1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  //1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  //1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  //0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  //0.0f, 0.0f,
	};
	const float m_left_verticies[18] = {
		-0.5f,  0.5f,  0.5f, // 1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, // 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, // 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, // 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, // 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, // 1.0f, 0.0f,
	};
	const float m_right_verticies[18] = {
		 0.5f,  0.5f,  0.5f, // 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, // 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, // 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, // 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f, // 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, // 1.0f, 0.0f,
	};
	const float m_bottom_verticies[18] = {
		-0.5f, -0.5f, -0.5f, // 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, // 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f, // 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, // 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f, // 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, // 0.0f, 1.0f,
	};
	const float m_top_verticies[18] = {
		-0.5f,  0.5f, -0.5f, // 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, // 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, // 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, // 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, // 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, // 0.0f, 1.0f
	};
};
