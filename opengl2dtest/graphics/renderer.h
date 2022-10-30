#pragma once
#include "renderer_resources.h"
#include "shader.h"
#include "texture.h"
#include "quad.h"
#include "camera.h"
#include "cube.h"

struct Renderer
{
	ShaderProgram shaders[SHADERS_LAST+1];
	Texture textures[TEXTURES_LAST+1];

	glm::mat4 projection_perspective;
	glm::mat4 projection_ortho;
};

Renderer renderer_create();
void renderer_update(Renderer* self, glm::mat4 perspective, glm::mat4 orthographic);
void renderer_render_quad(Renderer* self, glm::mat4 view, Quad* quad);
void renderer_render_cube(Renderer* self, glm::mat4 view, Cube* cube);
void renderer_render_custom(Renderer* self, glm::mat4 view, TextureType texture_type, ShaderType shader_type, int vao, int indicies, glm::vec3 position, glm::vec3 scale);
