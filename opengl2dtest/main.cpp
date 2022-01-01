#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_m.h"
#include "camera.h"
#include "SimplexNoise.h"
#include "block_vertex_builder.h"
#include "blocks/block.h"

#include <iostream>
#include <tuple>
#include <unordered_map>
#include <map>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);
GLFWwindow* init_and_create_window();
std::tuple<GLuint, GLuint> load_textures();

// settings
constexpr unsigned int SCR_WIDTH = 1280;
constexpr unsigned int SCR_HEIGHT = 960;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

block* blocks;
int chunk_size_x = 16;
int chunk_size_y = 16;
int chunk_size_z = 16;
int blocks_in_chunk = chunk_size_x * chunk_size_y * chunk_size_z;
int chunk_draw_distance = 12;

// camera
Camera camera(glm::vec3(0.0f, chunk_size_y + 10, 0.0f));

int normalize(int height)
{
	return 0;
}

int offset(int x, int y, int z)
{
	return (x * chunk_size_x) + (y * chunk_size_y) + z;
}

void generate_noise(block_vertex_builder* bvb, glm::vec3* pos)
{
	for (int x = 0; x < chunk_size_x; x++)
	{
		for (int y = 0; y < chunk_size_y; y++)
		{
			for (int z = 0; z < chunk_size_z; z++)
			{
				bvb->m_blocks[offset(x, y, z)].type = block_type::DIRT_GRASS;
			}
		}
	}
}

std::vector<std::pair<glm::vec3, block_vertex_builder>> chunks;

void init_chunks()
{
	for (int x = (chunk_draw_distance / 2) * -1; x < chunk_draw_distance; x++)
	{
		for (int z = (chunk_draw_distance / 2) * -1; z < chunk_draw_distance; z++)
		{
			block_vertex_builder bvb{ blocks_in_chunk, chunk_size_x, chunk_size_y, chunk_size_z };

			bvb.m_blocks = new block[blocks_in_chunk];
			auto key = glm::vec3(x * chunk_size_x, 0, z * chunk_size_z);
			chunks.push_back(std::make_pair(key, bvb));
		}
	}


	for (int i = 0; i < chunks.size(); i++)
	{
		auto* chunk = &chunks[i];

		generate_noise(&chunk->second, &chunk->first);
		chunk->second.build_mesh();
		chunk->second.setup_buffers();
	}
}

int main()
{
	auto window = init_and_create_window();

	// build and compile our shader zprogram
	Shader ourShader("7.4.camera.vs",
		"7.4.camera.fs");

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

		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

		for (auto& it : chunks)
		{
			ourShader.setMat4("model", glm::translate(glm::mat4(1.0f), it.first));
			it.second.draw();
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

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}

	glEnable(GL_DEPTH_TEST);

	return window;
}

unsigned char* load_png(const char* path)
{
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	return stbi_load(path, &width, &height, &nrChannels, 0);
}

std::tuple<GLuint, GLuint> load_textures()
{
	unsigned int texture2;
	unsigned char* dirt = load_png("resources\\textures\\dirt.png");
	unsigned char* dirt_grass_side = load_png("resources\\textures\\dirt_grass_side.png");
	unsigned char* dirt_grass_top = load_png("resources\\textures\\dirt_grass_top.png");
	unsigned char* stone = load_png("resources\\textures\\stone.png");

	unsigned int texture1;
	GLsizei width = 48;
	GLsizei height = 48;
	GLsizei layerCount = 6;
	GLsizei mipLevelCount = 1;

	glGenTextures(1, &texture1);
	auto l = glGetError();
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture1);
	auto k = glGetError();
	// Allocate the storage.
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGB8, 48, 48, layerCount);
	auto a1 = glGetError();
	// Upload pixel data.
	// The first 0 refers to the mipmap level (level 0, since there's only 1)
	// The following 2 zeroes refers to the x and y offsets in case you only want to specify a subrectangle.
	// The final 0 refers to the layer index offset (we start from index 0 and have 2 levels).
	// Altogether you can specify a 3D box subset of the overall texture, but only one mip level at a time.
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, stone);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, dirt);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 2, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, dirt_grass_side);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 3, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, dirt_grass_top);
	// MIPMAPS???
	////if (!c && !b && !i)
	//	std::cout << "\ntexture initialized correctly!";
	////else
	//	std::cout << "\ntexture initialization failed!\n";
	//glGenTextures(1, &texture1);	return std::make_tuple(texture1, texture2);

	//// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_LOD_BIAS, -1);

	return std::make_tuple(texture1, texture2);
}

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
