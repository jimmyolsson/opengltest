#pragma once
#include "robin_hood.h"
#include <glm/glm.hpp>
struct Chunk;
typedef robin_hood::unordered_flat_map<glm::ivec2, Chunk> chunk_map_t;
