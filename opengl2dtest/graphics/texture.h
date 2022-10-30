#pragma once

struct Texture
{
	unsigned int handle;
	int type;
};

Texture texture_create(const char* path);
Texture texture_atlas_create(int count, const char* paths, ...);
