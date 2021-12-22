#pragma once
#include "block_vertex_builder.h"

#include <vector>

class chunk 
{
public:
	chunk(int chunk_count)
	{
	}

	std::vector<block_vertex_builder> chunks;
private:
};
