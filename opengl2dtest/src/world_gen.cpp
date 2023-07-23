#include "world_gen.h"
#include "chunk.h"
#include "util/common.h"
#include <PerlinNoise/PerlinNoise.hpp>
#include "spline.h"

static int world_width = 0;
static int world_height = 0;

// TODO: move to util func
static int to_1d_array(short x, short y, short z)
{
	return (z * world_width * world_height) + (y * world_width) + x;
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

double linear_map(double x, double x1, double y1, double x2, double y2)
{
	double m = (y2 - y1) / (x2 - x1);
	double b = y1 - m * x1;
	return m * x + b;
}

static siv::PerlinNoise::seed_type seed = 123457u;
static siv::PerlinNoise perlin{ seed };

static std::vector<double> X = { -1.0, -0.5, 0,   0.5, 1 };
static std::vector<double> Y = { 50.0, 50.0, 100, 150, 255 };
static tk::spline s(X,Y);

void add_spline_point(double x, double y)
{
	X.push_back(x);
	Y.push_back(y);
}

BlockType get_block_type(int x, int y, int z)
{
	int terrain_height = 0;
	double xx = x * 0.01f;
	double zz = z * 0.01f;

	s.make_monotonic();
	double noise = perlin.octave2D_11(xx, zz, 4);

	double noise_splined = s(noise);

	int surfaceY = noise_splined;
	return y < surfaceY ? BlockType::STONE : BlockType::AIR;
}

void generate_world_noise(block* blocks, float* noisee, const int xoffset, const int zoffset)
{
	for (int z = 0; z < world_width; z++)
	{
		for (int x = 0; x < world_width; x++)
		{
			for (int y = 0; y < world_height; y++)
			{
				int index = to_1d_array(x, y, z);
				blocks[index].type = get_block_type(x + xoffset, y, z + zoffset);
			}
		}
	}

	static const int sea_level = 70;
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
					else
						blocks[index].type = BlockType::SAND;
				}
				else
				{
					if (blocks[to_1d_array(x, y, z)].type == BlockType::STONE)
					{
						if (blocks[to_1d_array(x, y + 1, z)].type == BlockType::AIR)
						{
							blocks[to_1d_array(x, y, z)].type = BlockType::DIRT_GRASS;
						}
						for (int i = 0; i < 10; i++)
						{
							if(!(i + y > world_height) && blocks[to_1d_array(x, y+i, z)].type != BlockType::AIR && blocks[to_1d_array(x, y+i, z)].type != BlockType::DIRT_GRASS)
								blocks[to_1d_array(x, y, z)].type = BlockType::DIRT;
						}
					}
				}
			}
		}
	}
}
void generate_world_flatgrass(block* blocks, const int xoffset, const int zoffset)
{
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
					blocks[index].type = BlockType::DIRT;
				}
			}
		}
	}
}

void world_generate(block* blocks, float* noise, const int xoffset, const int zoffset, const int width, const int height)
{
	world_width = width;
	world_height = height;

	for (int z = 0; z < world_width; z++)
	{
		for (int y = 0; y < world_height; y++)
		{
			for (int x = 0; x < world_width; x++)
			{
				int index = to_1d_array(x, y, z);
				blocks[index].type = BlockType::AIR;
			}
		}
	}

	//generate_world_cube(blocks, xoffset, zoffset);
	//generate_world_flatgrass(blocks, xoffset, zoffset);
	generate_world_noise(blocks, noise, xoffset, zoffset);
}

// TODO:
/* ----------Move this generation stuff somewhere else ---------- */

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

#include <vector>
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

