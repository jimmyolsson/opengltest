#pragma once
#include "robin_hood.h"
#include <glm/glm.hpp>
struct chunk;
typedef robin_hood::unordered_flat_map<glm::ivec2, chunk> chunk_map_t;
