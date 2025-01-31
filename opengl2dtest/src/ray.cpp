#include "ray.h"

#include <glm/gtx/norm.hpp>

#include "robin_hood.h"
static inline glm::vec3 intbound(glm::vec3 s, glm::vec3 ds)
{
	glm::vec3 res;
	for (size_t i = 0; i < 3; i++)
	{
		res[i] =
			(ds[i] > 0 ?
				(glm::ceil(s[i]) - s[i])
				: (s[i] - glm::floor(s[i])))
			/ glm::abs(ds[i]);
	}
	return res;
}

ray_hit_result ray_fire(Ray* self, float max_distance, chunk_map_t* chunks)
{
	ray_hit_result result;
	glm::ivec3 p, step;
	glm::vec3 t_max, t_delta;
	float radius;

	p = glm::floor(self->origin);
	step = glm::sign(self->direction);
	t_max = intbound(self->origin, self->direction);
	t_delta = glm::vec3(step) / self->direction;
	radius = max_distance / glm::l2Norm(self->direction);
	glm::ivec3 d = glm::ivec3(0);

	while (true)
	{
		//Check if we hit something, return block if we did
		for (auto& it : *chunks)
		{
			// If we're in the bounds of the chunk
			if (p.x < it.first.x + CHUNK_SIZE_WIDTH && p.x >= it.first.x &&
				p.z < it.first.y + CHUNK_SIZE_WIDTH && p.z >= it.first.y)
			{
				const int x = p.x - it.first.x;
				const int y = p.y;
				const int z = p.z - it.first.y;


				const auto a = chunk_get_block(&it.second, x, y, z);
				if (a.b->type != BlockType::AIR && a.b->type != BlockType::WATER)
				{
					// Inside the intersect_block method, after finding a hit block:
					float intersection_time = t_max.x < t_max.y ? (t_max.x < t_max.z ? t_max.x : t_max.z) : (t_max.y < t_max.z ? t_max.y : t_max.z);
					glm::vec3 intersection_point = self->origin + intersection_time * self->direction;
					if (d.x == 1)
					{ // If the hit face is in the positive X direction
						float local_y = intersection_point.y - glm::floor(intersection_point.y);
						if (local_y < 0.5f)
						{
							// Intersection is in the bottom half of the face
							result.top_half = false;
						}
						else
						{
							// Intersection is in the top half of the face
							result.top_half = true;
						}
					}

					result.block_pos = glm::ivec3(x, y, z);
					result.chunk_hit = &it.second;
					result.chunk_world_pos = it.first;
					result.direction = d;
					return result;
				}
			}
		}

		if (t_max.x < t_max.y)
		{
			if (t_max.x < t_max.z)
			{
				if (t_max.x > radius)
				{
					break;
				}

				p.x += step.x;
				t_max.x += t_delta.x;
				d = glm::ivec3(-step.x, 0, 0);
			}
			else
			{
				if (t_max.z > radius)
				{
					break;
				}

				p.z += step.z;
				t_max.z += t_delta.z;
				d = glm::ivec3(0, 0, -step.z);
			}
		}
		else
		{
			if (t_max.y < t_max.z)
			{
				if (t_max.y > radius)
				{
					break;
				}

				p.y += step.y;
				t_max.y += t_delta.y;
				d = glm::ivec3(0, -step.y, 0);
			}
			else
			{
				if (t_max.z > radius)
				{
					break;
				}

				p.z += step.z;
				t_max.z += t_delta.z;
				d = glm::ivec3(0, 0, -step.z);
			}
		}
	}

	result.chunk_hit = nullptr;
	return result;
}
