#include "texture.h"

#include "glad/glad.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <Windows.h>
#include <vector>
#include <string>
#include <algorithm>
#include "../util/common.h"

Texture texture_create(const char* path)
{
	Texture t;
	t.type = GL_TEXTURE_2D;

	glGenTextures(1, &t.handle);
	glBindTexture(GL_TEXTURE_2D, t.handle);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(data);

	return t;
}

unsigned char* load_png(const char* path, int* width, int* height, int* format)
{
	int channels;
	unsigned char* pixels;
	pixels = stbi_load(path, width, height, &channels, 0);
	if (channels == 3)
	{
		*format = GL_RGB;
	}
	else if (channels == 4)
	{
		*format = GL_RGBA;
	}

	return pixels;
}

#include <iostream>
Texture texture_atlas_create(int count, const char* paths, ...)
{
	Texture texture;
	texture.type = GL_TEXTURE_2D_ARRAY;
	
	va_list valist;
	va_start(valist, paths);

	GLsizei width = 16;
	GLsizei height = 16;

	GLsizei layerCount = count;
	GLsizei mipLevelCount = 4;

	glGenTextures(1, &texture.handle);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture.handle);

	// Allocate the storage.
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGBA8, width, height, layerCount);

	const char* path = paths;
	for(int i = 0; i < count; i++)
	{
		int width, height, format;
		unsigned char* pixels = load_png(path, &width, &height, &format);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, format, GL_UNSIGNED_BYTE, pixels);

		g_logger_debug("Loading textures from: %s - index: %d", path, i);
		path = va_arg(valist, const char*);
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	va_end(valist);

	return texture;
}

Texture texture_cubemap_create(const char* path)
{
	static const std::vector<std::string> faces
	{
		"resources\\textures\\skybox\\right.png",
		"resources\\textures\\skybox\\left.png",
		"resources\\textures\\skybox\\top.png",
		"resources\\textures\\skybox\\bottom.png",
		"resources\\textures\\skybox\\front.png",
		"resources\\textures\\skybox\\back.png",
	};

	Texture texture;
	texture.type = GL_TEXTURE_CUBE_MAP;
	glGenTextures(1, &texture.handle);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture.handle);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return texture;
}