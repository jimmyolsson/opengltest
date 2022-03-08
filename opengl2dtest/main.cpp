#ifdef __EMSCRIPTEN__
#include <GL/gl.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>
#else
#include <glad/glad.h>
#endif

#ifdef __EMSCRIPTEN__
//#include <GL/glfw.h>
#include <GLFW/glfw3.h>
#else
#include <GLFW/glfw3.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#ifdef __EMSCRIPTEN__
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#else
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/norm.hpp>
#endif

#include "shader_m.h"
#include "camera.h"
#include "chunk.h"
#include "blocks/block.h"

#include <iostream>
#include <tuple>
#include <algorithm>
#include <execution>
#include <cmath>
#include <array>
#include <chrono>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);
GLFWwindow* init_and_create_window();
std::tuple<GLuint, GLuint> load_textures();

// settings
constexpr unsigned int SCR_WIDTH = 1360;
constexpr unsigned int SCR_HEIGHT = 960;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

block* blocks;
const int blocks_in_chunk = CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT * CHUNK_SIZE_WIDTH;

#include "noise2.h"
#include "robin_hood.h"
// camera
//Camera camera(glm::vec3(20.0f, 45.0f, 150.0f));
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));

//static std::unordered_map<glm::ivec2, chunk> chunks;
//static std::unordered_map<glm::ivec2, int> chunks;
static robin_hood::unordered_node_map<glm::ivec2, chunk> chunks = {};

int offset(int x, int y, int z)
{
	return (z * CHUNK_SIZE_WIDTH * CHUNK_SIZE_HEIGHT) + (y * CHUNK_SIZE_WIDTH) + x;
}

float mapRange(float val, float inMin, float inMax, float outMin, float outMax)
{
	return (val - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

float redistribute(float noise, int exponent, int modifier)
{
	return pow(noise * modifier, exponent);
}

siv::PerlinNoise perlin;
void generate_noise(chunk* chunk, int xoffset, int zoffset)
{
	const float zoom = 0.01;
	const float persistence = 0.5;
	const int octaves = 3;
	const int exponent = 6;
	const float modifier = 1.2;

	for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
	{
		for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
		{
			float xi = x + xoffset;
			float zi = z + zoffset;

			xi *= zoom;
			zi *= zoom;

			xi += zoom;
			zi += zoom;

			float noise = perlin.octave2D_01(xi, zi, octaves, persistence);
			noise = redistribute(noise, octaves, modifier);
			noise = mapRange(noise, 0, 1, 1, CHUNK_SIZE_HEIGHT);

			int height = noise;

			for (int y = 0; y < height; y++)
			{
				chunk->blocks[offset(x, y, z)].type = block_type::DIRT_GRASS;
			}
		}
	}

	int sea_level = 5;
	for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
	{
		for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
		{
			for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
			{
				block* current = &chunk->blocks[offset(x, y, z)];

				if (y <= sea_level)
				{
					if (current->type == block_type::AIR)
					{
						current->type = block_type::WATER;
					}
					else if (current->type != block_type::AIR)
					{
						current->type = block_type::SAND;
					}
				}
			}
		}
	}
}

void create_and_init_chunk(int x, int z)
{
	chunk chunk;
	chunk.blocks = new block[blocks_in_chunk];
	chunk.chunks = &chunks;
	auto pos = glm::vec2(x * CHUNK_SIZE_WIDTH, z * CHUNK_SIZE_WIDTH);

	generate_noise(&chunk, pos.x, pos.y);
	chunks[pos] = chunk;
}

void init_chunks()
{
	std::cout << "Generate chunks\n";

	auto start_noise_gen = std::chrono::steady_clock::now();
	for (int x = (CHUNK_DRAW_DISTANCE / 2) * -1; x < CHUNK_DRAW_DISTANCE / 2; x++)
	{
		for (int z = (CHUNK_DRAW_DISTANCE / 2) * -1; z < CHUNK_DRAW_DISTANCE / 2; z++)
		{
			create_and_init_chunk(x, z);
		}
	}
	auto end_noise_gen = std::chrono::steady_clock::now();
	std::cout << "Generating chunks took: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_noise_gen - start_noise_gen).count() << "ms\n";

	std::cout << "Generating meshes and creating buffers\n";
	auto start_meshgen = std::chrono::steady_clock::now();
	int counter = 0;
	std::for_each(std::execution::par_unseq, std::begin(chunks), std::end(chunks),
		[&](auto& iter)
		{
			ChunkPrivate::generate_mesh(iter.second, iter.first);
		});
	for (auto& iter : chunks)
	{
		// Calls to opengl has to be single threaded :(
		ChunkPrivate::init_buffers(iter.second, counter);
	}
	//for (auto& iter : chunks)
	//{
	//	ChunkPrivate::generate_mesh(iter.second, iter.first);
	//	ChunkPrivate::init_buffers(iter.second);
	//}
	auto end_meshgen = std::chrono::steady_clock::now();
	std::cout << "Generating meshes and buffers took: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_meshgen - start_meshgen).count() << "ms\n";
}

int main()
{
	auto window = init_and_create_window();

	// build and compile our shader zprogram
	Shader ourShader("resources\\7.4.camera.vs",
		"resources\\7.4.camera.shader");

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

	const auto [texture1, texture2] = load_textures();

	ourShader.use();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	init_chunks();

	//for (auto& a : chunks)
	//{
	//	//delete[] a.second.blocks;
	//	delete[] a.second.gpu_data_arr;
	//}
	glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	//robin_hood::unordered_node_map<glm::ivec2, chunk> tmp = {};
	//chunks.swap(tmp);

	int nbFrames = 0;
	double lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;

		// measure frames
		nbFrames++;
		if (currentTime - lastTime >= 1.0)
		{
			double msPerFrame = 1000.0 / double(nbFrames);
			double ms = 1000 / msPerFrame;
			//std::cout << msPerFrame << "ms/frame  |  " << 1 / (msPerFrame / 1000) << " FPS" << '\n';
			nbFrames = 0;
			lastTime += 1.0;
		}

		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture1);

		// pass projection matrix to shader (note that in this case it could change every frame)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		ourShader.setMat4("projection", projection);

		// camera/view transformation
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("view", view);

		// Draw all chunks
		for (auto& iter : chunks)
		{
			ourShader.setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(iter.first.x, 0, iter.first.y)));
			ChunkPrivate::draw(iter.second);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

GLFWwindow* init_and_create_window()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "opengltest", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

#ifndef __EMSCRIPTEN__
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}
#endif

	glEnable(GL_DEPTH_TEST);

	return window;
}

unsigned char* load_png(const char* path)
{
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	std::cout << nrChannels << "\n";
	return data;
}

std::tuple<GLuint, GLuint> load_textures()
{
	unsigned int texture2;
	unsigned char* dirt = load_png("resources\\dirt.png");
	unsigned char* dirt_grass_side = load_png("resources\\dirt_grass_side.png");
	unsigned char* dirt_grass_top = load_png("resources\\dirt_grass_top.png");
	unsigned char* stone = load_png("resources\\stone.png");
	unsigned char* sand = load_png("resources\\sand.png");
	unsigned char* water = load_png("resources\\water.png");
	unsigned char* leaves = load_png("resources\\leaves.png");

	unsigned int texture1;
	GLsizei width = 48;
	GLsizei height = 48;
	GLsizei layerCount = 7;
	GLsizei mipLevelCount = 1;

	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture1);
	// Allocate the storage.
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_LOD_BIAS, -1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGBA8, 48, 48, layerCount);
	// Upload pixel data.
	// The first 0 refers to the mipmap level (level 0, since there's only 1)
	// The following 2 zeroes refers to the x and y offsets in case you only want to specify a subrectangle.
	// The final 0 refers to the layer index offset (we start from index 0 and have 2 levels).
	// Altogether you can specify a 3D box subset of the overall texture, but only one mip level at a time.
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, stone);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, dirt);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 2, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, dirt_grass_side);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 3, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, dirt_grass_top);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 4, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, sand);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 5, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, water);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 6, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, leaves);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	return std::make_tuple(texture1, texture2);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//glm::vec3 to_voxel_pos(const glm::vec3 pos)
//{
//	for (auto& it : chunks)
//	{
//		if (pos.x > it.first.x && pos.x < it.first.x + CHUNK_SIZE_WIDTH)
//		{
//			it.second.blocks[to_1d_array(pos.x + it.first.x, pos.y, pos.z)].type == block_type::DIRT_GRASS;
//			ChunkPrivate::generate_mesh(it.second, it.first);
//		}
//	}
//	return glm::vec3(1, 1, 1);
//}

// find the smallest possible t such that s + t * ds is an integer
inline glm::vec3 intbound(glm::vec3 s, glm::vec3 ds)
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



struct Ray {
	glm::vec3 origin, direction;

	Ray() = default;
	Ray(const glm::vec3& o, const glm::vec3& d)
		: origin(o), direction(d)
	{
	}

	void intersect_block(float max_distance)
	{
		//util::Direction d = 0;
		glm::ivec3 p, step;
		glm::vec3 t_max, t_delta;
		float radius;

		p = glm::floor(this->origin);
		step = glm::sign(this->direction);
		t_max = intbound(this->origin, this->direction);
		t_delta = glm::vec3(step) / this->direction;
		radius = max_distance / glm::l2Norm(this->direction);

		while (true)
		{
			//std::cout << "Ray: x:" << p.x << " y: " << p.y << " z: " << p.z << "\n";
			//Check if we hit something, return block if we did
			for (auto& it : chunks)
			{
				if (p.x < it.first.x + CHUNK_SIZE_WIDTH && p.x > it.first.x &&
					p.z < it.first.y + CHUNK_SIZE_WIDTH && p.z > it.first.y)
				{
					int x = p.x - it.first.x;
					int y = p.y;
					int z = p.z - it.first.y;

					auto& a = it.second.blocks[to_1d_array(x, y, z)];
					int b = 2;
					if (a.type == block_type::AIR)
					{
						it.second.blocks[to_1d_array(p.x, p.y, p.z)].type = block_type::STONE;
						ChunkPrivate::generate_mesh(it.second, it.first);
						//ChunkPrivate::init_buffers(it.second);
					}

					//std::string dick = "";
					//switch (a.type)
					//{
					//case block_type::AIR:
					//	dick = "AIR";
					//	break;
					//case block_type::SAND:
					//	dick = "SAND";
					//	break;
					//case block_type::STONE:
					//	dick = "STONE";
					//	break;
					//case block_type::DIRT_GRASS:
					//	dick = "DIRT_GRASS";
					//	break;
					//case block_type::DIRT:
					//	dick = "DIRT";
					//	break;
					//default:
					//	dick = "DEFAULT";
					//}
					//std::cout << "-------NEW RAY-------\n";
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
					//d = glm::ivec3(-step.x, 0, 0);
				}
				else
				{
					if (t_max.z > radius)
					{
						break;
					}

					p.z += step.z;
					t_max.z += t_delta.z;
					//d = glm::ivec3(0, 0, -step.z);
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
					//d = glm::ivec3(0, -step.y, 0);
				}
				else
				{
					if (t_max.z > radius)
					{
						break;
					}

					p.z += step.z;
					t_max.z += t_delta.z;
					//d = glm::ivec3(0, 0, -step.z);
				}
			}
		}
	}
};

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		// viewport, 0 top of screen
		double x;
		double y;
		glfwGetCursorPos(window, &x, &y);

		auto position = camera.Position;
		auto rotation = camera.Front;

		//Ray r(camera.Position, glm::vec3(camera.Pitch, camera.Yaw, 0));
		//r.intersect_block(4);
	}
}
