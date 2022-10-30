#include "renderer.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

void renderer_update(Renderer* self, glm::mat4 perspective, glm::mat4 orthographic)
{
	self->projection_perspective = perspective;
	self->projection_ortho = orthographic;
}

Renderer renderer_create()
{
	Renderer renderer;

	renderer.shaders[SHADER_CHUNK] = shader_create("..\\resources\\shaders\\opaque_vert.glsl", "..\\resources\\shaders\\opaque_frag.glsl");
	renderer.shaders[SHADER_OUTLINE] = shader_create("..\\resources\\shaders\\outline_vert.glsl", "..\\resources\\shaders\\outline_frag.glsl");
	renderer.shaders[SHADER_BASIC_TEXTURE] = shader_create("..\\resources\\shaders\\basic_texture_vert.glsl", "..\\resources\\shaders\\basic_texture_frag.glsl");
	renderer.shaders[SHADER_BASIC_COLOR] = shader_create("..\\resources\\shaders\\basic_color_vert.glsl", "..\\resources\\shaders\\basic_color_frag.glsl");

	renderer.textures[TEXTURE_ATLAS_CHUNK] = texture_atlas_create(8,
		"..\\resources\\textures\\block\\stone.png",
		"..\\resources\\textures\\block\\dirt.png",
		"..\\resources\\textures\\block\\dirt_grass_side.png",
		"..\\resources\\textures\\block\\dirt_grass_top.png",
		"..\\resources\\textures\\block\\sand.png",
		"..\\resources\\textures\\block\\leaves.png",
		"..\\resources\\textures\\block\\oak_log.png",
		"..\\resources\\textures\\block\\oak_log_top.png"
	);

	renderer.textures[TEXTURE_UI_CROSSHAIR] = texture_create("..\\resources\\textures\\gui\\crosshair.png");
	renderer.textures[TEXTURE_UI_TOOLBAR] = texture_create("..\\resources\\textures\\gui\\toolbar.png");
	renderer.textures[TEXTURE_UI_TOOLBAR_HIGHLIGHT] = texture_create("..\\resources\\textures\\gui\\toolbar_highlight.png");

	return renderer;
}

void renderer_render_quad(Renderer* self, glm::mat4 view, Quad* quad)
{
	ShaderProgram* shader = nullptr;
	Texture* texture = nullptr;
	if (quad->texture_type == TEXTURE_NONE)
	{
		shader = &self->shaders[SHADER_BASIC_COLOR];
	}
	else
	{
		shader = &self->shaders[SHADER_BASIC_TEXTURE];
		texture = &self->textures[quad->texture_type];

		glActiveTexture(GL_TEXTURE0 + texture->handle);
		glBindTexture(texture->type, texture->handle);
	}

	shader_use(shader);
	shader_set_mat4(shader, "projection", self->projection_ortho);
	shader_set_mat4(shader, "view", view);

	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(quad->position, 1));
	model = glm::scale(model, glm::vec3(quad->scale, 0));
	shader_set_mat4(shader, "model", model);

	if (texture != nullptr)
		shader_set_int(shader, "texture1", texture->handle);

	glBindVertexArray(quad->vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	if(texture != nullptr)
		glBindTexture(texture->type, 0);
}

void renderer_render_cube(Renderer* self, glm::mat4 view, Cube* cube)
{
	ShaderProgram* shader = &self->shaders[cube->shader_type];
	Texture* texture = &self->textures[cube->texture_type];

	shader_use(shader);
	shader_set_mat4(shader, "projection", self->projection_perspective);
	shader_set_mat4(shader, "view", view);

	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, cube->position);
	model = glm::scale(model, cube->scale);
	shader_set_mat4(shader, "model", model);

	glActiveTexture(GL_TEXTURE0 + texture->handle);
	glBindTexture(texture->type, texture->handle);	
	
	glBindVertexArray(cube->vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
	glBindTexture(texture->type, 0);
}

void renderer_render_custom(Renderer* self, glm::mat4 view, TextureType texture_type, ShaderType shader_type, int vao, int indicies, glm::vec3 position, glm::vec3 scale)
{
	ShaderProgram* shader = &self->shaders[shader_type];
	Texture* texture = &self->textures[texture_type];

	shader_use(shader);
	shader_set_mat4(shader, "projection", self->projection_perspective);
	shader_set_mat4(shader, "view", view);

	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, position);
	model = glm::scale(model, scale);
	shader_set_mat4(shader, "model", model);

	glActiveTexture(GL_TEXTURE0 + texture->handle);
	glBindTexture(texture->type, texture->handle);	
	
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, indicies);

	glBindVertexArray(0);
	glBindTexture(texture->type, 0);
}
