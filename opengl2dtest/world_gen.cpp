#include "world_gen.h"
#include "chunk.h"
#include "util/common.h"

int world_width = 0;
int world_height = 0;

// TODO: move to util func
static int to_1d_array(short x, short y, short z)
{
	return (z * world_width * world_height) + (y * world_width) + x;
}

static FastNoise::SmartNode<> asdnoise = FastNoise::NewFromEncodedNodeTree("EQACAAAAAAAgQBAAAAAAQBkAEwDD9Sg/DQAEAAAAAAAgQAkAAGZmJj8AAAAAPwEEAAAAAAAAAEBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAM3MTD4AMzMzPwAAAAA/");

void generate_world_cube(block* blocks, const int xoffset, const int zoffset);
void generate_world_noise(block* blocks, memory_arena* pool, const int xoffset, const int zoffset)
{
	if (xoffset == -128 && zoffset == -128)
	{
		generate_world_cube(blocks, xoffset, zoffset);
		return;
	}
	const float frequency = 0.0050f;
	const float threshold = 0.01f;

	float* noise = (float*)memory_arena_get(pool);

	TIMER_START(WORLD_GEN_GENUNIFORMGRID);
	auto min_max = asdnoise.get()->GenUniformGrid3D(noise, xoffset, -world_height / 2, zoffset, world_width, world_height, world_width, frequency, 1338);
	TIMER_END(WORLD_GEN_GENUNIFORMGRID);

	for (int i = 0; i < world_width * world_height * world_width; i++)
		noise[i] *= -1;

	const static int sea_level = 130;
	for (int z = 0; z < world_width; z++)
	{
		for (int y = 0; y < world_height; y++)
		{
			for (int x = 0; x < world_width; x++)
			{
				int index = to_1d_array(x, y, z);
				if (noise[index] > threshold)
				{
					if (noise[to_1d_array(x, y + 2, z)] > threshold)
					{
						blocks[index].type = BlockType::STONE;
					}
					else if (noise[to_1d_array(x, y + 1, z)] > threshold)
					{
						blocks[index].type = BlockType::DIRT;
					}
					else
					{
						blocks[index].type = BlockType::DIRT_GRASS;
					}
				}
			}
		}
	}

	for (int z = 0; z < world_width; z++)
	{
		for (int y = 0; y < world_height; y++)
		{
			for (int x = 0; x < world_width; x++)
			{
				int index = to_1d_array(x, y, z);
				if (y < sea_level)
				{
					if (blocks[index].type == BlockType::AIR)
					{
						blocks[index].type = BlockType::WATER;
					}
					else if (blocks[index].type == BlockType::DIRT_GRASS || blocks[index].type == BlockType::DIRT)
						blocks[index].type = BlockType::SAND;
				}
			}
		}
	}
}

void generate_world_cube(block* blocks, const int xoffset, const int zoffset)
{
	for (int z = 0; z < world_width; z++)
	{
		for (int y = 0; y < world_height; y++)
		{
			for (int x = 0; x < world_width; x++)
			{
				int index = to_1d_array(x, y, z);
				blocks[index].type = BlockType::DIRT;
			}
		}
	}
}
void generate_world_flatgrass(block* blocks, const int xoffset, const int zoffset)
{
	if (world_height == 1) {
		int index = to_1d_array(0, 0, 0);
		blocks[index].type = BlockType::DIRT_GRASS;
		return;
	}
	for (int z = 0; z < world_width; z++)
	{
		for (int y = 0; y < world_height; y++)
		{
			for (int x = 0; x < world_width; x++)
			{
				int index = to_1d_array(x, y, z);
				if (y < world_height / 6)
				{
					blocks[index].type = BlockType::DIRT_GRASS;
				}
				if (y < (world_height / 6) - 1)
				{
					blocks[index].type = BlockType::DIRT_GRASS;
				}
			}
		}
	}
}

void world_generate(block* blocks, memory_arena* pool, const int xoffset, const int zoffset, const int width, const int height)
{
	world_width = width;
	world_height = height;
#if _DEBUG
	// dosent work in debug otherwise..
	for (int z = 0; z < world_width; z++)
	{
		for (int y = 0; y < world_height; y++)
		{
			for (int x = 0; x < world_width; x++)
			{
				int index = to_1d_array(x, y, z);
				blocks[index].type = BlockType::AIR;
				blocks[index].sky = false;
			}
		}
	}
#endif // DEBUG

	//generate_world_cube(blocks, xoffset, zoffset);
	//generate_world_flatgrass(blocks, xoffset, zoffset);
	TIMER_START(WORLD_GEN);
	generate_world_noise(blocks, pool, xoffset, zoffset);
	TIMER_END(WORLD_GEN)
}

// TODO:
/* ----------Move this generation stuff somewhere else ---------- */

struct structure_node
{
	glm::ivec3 pos;
	BlockType type;
};

std::vector<structure_node> sphere;
// Function to put pixels
// at subsequence points
void drawCircle(int xc, int yc, int x, int y, int world_y, std::vector<structure_node>& sphere)
{
	sphere.push_back({ { xc + x, world_y, yc + y }, BlockType::STONE });
	sphere.push_back({ { xc - x, world_y, yc + y }, BlockType::STONE });
	sphere.push_back({ { xc + x, world_y, yc - y }, BlockType::STONE });
	sphere.push_back({ { xc - x, world_y, yc - y }, BlockType::STONE });
	sphere.push_back({ { xc + y, world_y, yc + x }, BlockType::STONE });
	sphere.push_back({ { xc - y, world_y, yc + x }, BlockType::STONE });
	sphere.push_back({ { xc + y, world_y, yc - x }, BlockType::STONE });
	sphere.push_back({ { xc - y, world_y, yc - x }, BlockType::STONE });
}

// Function for circle-generation
// using Bresenham's algorithm
void circleBres(int xc, int yc, int r, int world_y, std::vector<structure_node>& sphere)
{
	int x = 0, y = r;
	int d = 3 - 2 * r;
	drawCircle(xc, yc, x, y, world_y, sphere);
	while (y >= x)
	{
		// for each pixel we will
		// draw all eight pixels

		x++;

		// check for decision parameter
		// and correspondingly
		// update d, x, y
		if (d > 0)
		{
			y--;
			d = d + 4 * (x - y) + 10;
		}
		else
			d = d + 4 * x + 6;
		drawCircle(xc, yc, x, y, world_y, sphere);
	}
}

const int sphere_size = 16;
int map[sphere_size][sphere_size][sphere_size];

std::vector<structure_node> sphere_algo(int x0, int y0, int z0, int r)
{
	int col = 1;
	int n = sphere_size;
	for (int x = 0; x < sphere_size; x++)
		for (int y = 0; y < sphere_size; y++)
			for (int z = 0; z < sphere_size; z++)
			{
				map[x][y][z] = 0;
			}

	int x, y, z, xa, ya, za, xb, yb, zb, xr, yr, zr, xx, yy, zz, rr = r * r;
	// bounding box
	xa = x0 - r; if (xa < 0) xa = 0; xb = x0 + r; if (xb > n) xb = n;
	ya = y0 - r; if (ya < 0) ya = 0; yb = y0 + r; if (yb > n) yb = n;
	za = z0 - r; if (za < 0) za = 0; zb = z0 + r; if (zb > n) zb = n;
	// project xy plane
	for (x = xa, xr = x - x0, xx = xr * xr; x < xb; x++, xr++, xx = xr * xr)
		for (y = ya, yr = y - y0, yy = yr * yr; y < yb; y++, yr++, yy = yr * yr)
		{
			zz = rr - xx - yy; if (zz < 0) continue; zr = sqrt(zz);
			z = z0 - zr; if ((z > 0) && (z < n)) map[x][y][z] = col;
			z = z0 + zr; if ((z > 0) && (z < n)) map[x][y][z] = col;
		}
	// project xz plane
	for (x = xa, xr = x - x0, xx = xr * xr; x < xb; x++, xr++, xx = xr * xr)
		for (z = za, zr = z - z0, zz = zr * zr; z < zb; z++, zr++, zz = zr * zr)
		{
			yy = rr - xx - zz; if (yy < 0) continue; yr = sqrt(yy);
			y = y0 - yr; if ((y > 0) && (y < n)) map[x][y][z] = col;
			y = y0 + yr; if ((y > 0) && (y < n)) map[x][y][z] = col;
		}
	// project yz plane
	for (y = ya, yr = y - y0, yy = yr * yr; y < yb; y++, yr++, yy = yr * yr)
		for (z = za, zr = z - z0, zz = zr * zr; z < zb; z++, zr++, zz = zr * zr)
		{
			xx = rr - zz - yy; if (xx < 0) continue; xr = sqrt(xx);
			x = x0 - xr; if ((x > 0) && (x < n)) map[x][y][z] = col;
			x = x0 + xr; if ((x > 0) && (x < n)) map[x][y][z] = col;
		}

	std::vector<structure_node> a;
	for (int x = 0; x < sphere_size; x++)
		for (int y = 0; y < sphere_size; y++)
			for (int z = 0; z < sphere_size; z++)
			{
				if (map[x][y][z] != 0)
					a.push_back({ {x, y, z},{BlockType::STONE} });
			}
	return a;
}

std::vector<structure_node> tree_structure = {
	{{0, 1, 0}, BlockType::OAK_LOG},
	{{0, 2, 0}, BlockType::OAK_LOG},
	{{0, 3, 0}, BlockType::OAK_LOG},
	{{0, 4, 0}, BlockType::OAK_LOG},
	{{0, 5, 0}, BlockType::OAK_LOG},
	{{0, 6, 0}, BlockType::OAK_LOG},

	{{-2, 4, 1}, BlockType::LEAVES},
	{{-1, 4, 1}, BlockType::LEAVES},
	{{1, 4, 1}, BlockType::LEAVES},
	{{2, 4, 1}, BlockType::LEAVES},

	{{-2, 4, 0}, BlockType::LEAVES},
	{{-1, 4, 0}, BlockType::LEAVES},
	{{1, 4, 0}, BlockType::LEAVES},
	{{2, 4, 0}, BlockType::LEAVES},

	{{-2, 4, -1}, BlockType::LEAVES},
	{{-1, 4, -1}, BlockType::LEAVES},
	{{1, 4, -1}, BlockType::LEAVES},
	{{2, 4, -1}, BlockType::LEAVES},

	{{1, 4, -2}, BlockType::LEAVES},
	{{1, 4, -1}, BlockType::LEAVES},
	{{1, 4, 1}, BlockType::LEAVES},
	{{1, 4, 2}, BlockType::LEAVES},

	{{0, 4, -2}, BlockType::LEAVES},
	{{0, 4, -1}, BlockType::LEAVES},
	{{0, 4, 1}, BlockType::LEAVES},
	{{0, 4, 2}, BlockType::LEAVES},

	{{-1, 4, -2}, BlockType::LEAVES},
	{{-1, 4, -1}, BlockType::LEAVES},
	{{-1, 4, 1}, BlockType::LEAVES},
	{{-1, 4, 2}, BlockType::LEAVES},

	{{-2, 5, 1}, BlockType::LEAVES},
	{{-1, 5, 1}, BlockType::LEAVES},
	{{1, 5, 1}, BlockType::LEAVES},
	{{2, 5, 1}, BlockType::LEAVES},

	{{-2, 5, 0}, BlockType::LEAVES},
	{{-1, 5, 0}, BlockType::LEAVES},
	{{1, 5, 0}, BlockType::LEAVES},
	{{2, 5, 0}, BlockType::LEAVES},

	{{-2, 5, -1}, BlockType::LEAVES},
	{{-1, 5, -1}, BlockType::LEAVES},
	{{1, 5, -1}, BlockType::LEAVES},
	{{2, 5, -1}, BlockType::LEAVES},

	{{1, 5, -2}, BlockType::LEAVES},
	{{1, 5, -1}, BlockType::LEAVES},
	{{1, 5, 1}, BlockType::LEAVES},
	{{1, 5, 2}, BlockType::LEAVES},

	{{0, 5, -2}, BlockType::LEAVES},
	{{0, 5, -1}, BlockType::LEAVES},
	{{0, 5, 1}, BlockType::LEAVES},
	{{0, 5, 2}, BlockType::LEAVES},

	{{-1, 5, -2}, BlockType::LEAVES},
	{{-1, 5, -1}, BlockType::LEAVES},
	{{-1, 5, 1}, BlockType::LEAVES},
	{{-1, 5, 2}, BlockType::LEAVES},

	{{-1, 6, 1}, BlockType::LEAVES},
	{{1, 6, 1}, BlockType::LEAVES},

	{{-1, 6, 0}, BlockType::LEAVES},
	{{1, 6, 0}, BlockType::LEAVES},

	{{-1, 6, -1}, BlockType::LEAVES},

	{{1, 6, -1}, BlockType::LEAVES},

	{{1, 6, -1}, BlockType::LEAVES},
	{{1, 6, 1}, BlockType::LEAVES},

	{{0, 6, -1}, BlockType::LEAVES},
	{{0, 6, 1}, BlockType::LEAVES},

	{{-1, 6, -1}, BlockType::LEAVES},
	{{-1, 6, 1}, BlockType::LEAVES},

	{{0, 7, 0}, BlockType::LEAVES},
	{{-1, 7, 0}, BlockType::LEAVES},
	{{1, 7, 0}, BlockType::LEAVES},
	{{0, 7, -1}, BlockType::LEAVES},
	{{0, 7, 1}, BlockType::LEAVES},
};

