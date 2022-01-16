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
#endif

#include "shader_m.h"
#include "camera.h"
#include "chunk.h"
#include "blocks/block.h"

#include <iostream>
#include <tuple>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <execution>

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

#include <cmath>
#include <array>
#include "noise2.h"
#include "robin_hood.h"
// camera
Camera camera(glm::vec3(20.0f, 45.0f, 150.0f));
//Camera camera(glm::vec3(0.0f, 45.0f, 5.0f));

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

	for (int x = 0; x < CHUNK_SIZE_WIDTH; x++)
	{
		for (int y = 0; y < CHUNK_SIZE_HEIGHT; y++)
		{
			for (int z = 0; z < CHUNK_SIZE_WIDTH; z++)
			{
				block* current = &chunk->blocks[offset(x, y, z)];

				if (y <= 5)
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
				else
				{
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
	std::cout << "Generating chunks\n";

	for (int x = (CHUNK_DRAW_DISTANCE / 2) * -1; x < CHUNK_DRAW_DISTANCE; x++)
	{
		for (int z = (CHUNK_DRAW_DISTANCE / 2) * -1; z < CHUNK_DRAW_DISTANCE; z++)
		{
			create_and_init_chunk(x, z);
		}
	}

	const siv::PerlinNoise::seed_type seed = 123456u;
	siv::PerlinNoise perlin{ seed };
	for (auto& iter : chunks)
	{
		ChunkPrivate::generate_mesh(iter.second, iter.first);
		ChunkPrivate::init_buffers(iter.second);
	}

	std::cout << "Done!\n";
}

int main()
{
	auto window = init_and_create_window();

	// build and compile our shader zprogram
	Shader ourShader("resources\\7.4.camera.vs",
		"resources\\7.4.camera.shader");

	std::cout << glGetString(GL_VERSION) << "\n";

	const auto [texture1, texture2] = load_textures();

	ourShader.use();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	init_chunks();

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
			std::cout << msPerFrame << "ms/frame  |  " << 1 / (msPerFrame / 1000) << " FPS" << '\n';
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

	glfwTerminate();
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

	unsigned int texture1;
	GLsizei width = 48;
	GLsizei height = 48;
	GLsizei layerCount = 6;
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


	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGB8, 48, 48, layerCount);
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
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	return std::make_tuple(texture1, texture2);
}


//std::tuple<GLuint, GLuint> load_textures()
//{
//	unsigned int texture2;
//	unsigned char* dirt = load_png("resources\\dirt.png");
//	unsigned char* dirt_grass_side = load_png("resources\\dirt_grass_side.png");
//	unsigned char* dirt_grass_top = load_png("resources\\dirt_grass_top.png");
//
//	unsigned int texture1;
//	GLsizei width = 48;
//	GLsizei height = 48;
//	GLsizei layerCount = 6;
//	GLsizei mipLevelCount = 1;
//
//	glGenTextures(1, &texture1);
//	glBindTexture(GL_TEXTURE_2D, texture1);
//
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, dirt_grass_top);
//    glGenerateMipmap(GL_TEXTURE_2D);
//
//	return std::make_tuple(texture1, texture2);
//}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	glm::vec4 tmp = glm::vec4((xpos / SCR_WIDTH) * 2.0f - 1.0f, -((ypos / SCR_HEIGHT) * 2.0f - 1.0f), 0, 1.0f);
	glm::vec4 projectedScreen = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f) * tmp;
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
	}
}
