#include "renderer.h"
#include "../blocks/block.h"
#include "../util/common_graphics.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum ShaderUniform
{
	VIEW = 0,
	MODEL,
	PROJECTION
};

void _shader_uniform_mvp_cache(ShaderProgram* shader)
{
	shader->uniform_count = 3;
	shader->uniform_locations = (unsigned int*)malloc(shader->uniform_count * sizeof(unsigned int));

	shader->uniform_locations[ShaderUniform::VIEW] = glGetUniformLocation(shader->handle, "view");
	shader->uniform_locations[ShaderUniform::MODEL] = glGetUniformLocation(shader->handle, "model");
	shader->uniform_locations[ShaderUniform::PROJECTION] = glGetUniformLocation(shader->handle, "projection");
}

void _load_shaders(Renderer* self)
{
	g_logger_debug("Compiling shaders..");
#ifdef __EMSCRIPTEN__
	self->shaders[SHADER_CHUNK] = shader_create("resources/shaders/GLES/opaque_vert.glsl", "resources/shaders/GLES/opaque_frag.glsl");
	self->shaders[SHADER_OUTLINE] = shader_create("resources/shaders/GLES/outline_vert.glsl", "resources/shaders/GLES/outline_frag.glsl");
	self->shaders[SHADER_BASIC_TEXTURE] = shader_create("resources/shaders/GLES/basic_texture_vert.glsl", "resources/shaders/GLES/basic_texture_frag.glsl");
	self->shaders[SHADER_BASIC_COLOR] = shader_create("resources/shaders/GLES/basic_color_vert.glsl", "resources/shaders/GLES/basic_color_frag.glsl");
	self->shaders[SHADER_CUBEMAP] = shader_create("resources/shaders/GLES/cubemap_vert.glsl", "resources/shaders/GLES/cubemap_frag.glsl");
#else
	self->shaders[SHADER_CHUNK] = shader_create("resources/shaders/opaque_vert.glsl", "resources/shaders/opaque_frag.glsl");
	self->shaders[SHADER_OUTLINE] = shader_create("resources/shaders/outline_vert.glsl", "resources/shaders/outline_frag.glsl");
	self->shaders[SHADER_BASIC_TEXTURE] = shader_create("resources/shaders/basic_texture_vert.glsl", "resources/shaders/basic_texture_frag.glsl");
	self->shaders[SHADER_BASIC_COLOR] = shader_create("resources/shaders/basic_color_vert.glsl", "resources/shaders/basic_color_frag.glsl");
	self->shaders[SHADER_CUBEMAP] = shader_create("resources/shaders/cubemap_vert.glsl", "resources/shaders/cubemap_frag.glsl");
#endif

	// Do it like this for now. Maybe a better pattern will emerge later on?
	_shader_uniform_mvp_cache(&self->shaders[SHADER_CHUNK]);
	_shader_uniform_mvp_cache(&self->shaders[SHADER_OUTLINE]);
	_shader_uniform_mvp_cache(&self->shaders[SHADER_BASIC_COLOR]);
	_shader_uniform_mvp_cache(&self->shaders[SHADER_CUBEMAP]);

	{
		ShaderProgram* shader = &self->shaders[SHADER_BASIC_TEXTURE];

		shader->uniform_count = 4;
		shader->uniform_locations = (unsigned int*)malloc(shader->uniform_count * sizeof(unsigned int));
		shader->uniform_locations[ShaderUniform::VIEW] = glGetUniformLocation(shader->handle, "view");
		shader->uniform_locations[ShaderUniform::MODEL] = glGetUniformLocation(shader->handle, "model");
		shader->uniform_locations[ShaderUniform::PROJECTION] = glGetUniformLocation(shader->handle, "projection");
		shader->uniform_locations[3] = glGetUniformLocation(shader->handle, "texture1");
	}
}

void _load_textures(Renderer* self)
{
	// THIS HAS TO BE DEFINED IN THE SAME ORDER AS THE BlockTextureIndex ENUM
	self->textures[TEXTURE_ATLAS_CHUNK] = texture_atlas_create(BLOCK_TEXTURE_INDEX_LAST + 1,
		"resources/textures/block/stone.png",
		"resources/textures/block/dirt.png",
		"resources/textures/block/dirt_grass_side.png",
		"resources/textures/block/dirt_grass_top.png",
		"resources/textures/block/sand.png",
		"resources/textures/block/leaves.png",
		"resources/textures/block/oak_log.png",
		"resources/textures/block/oak_log_top.png",
		"resources/textures/block/water.png",
		"resources/textures/block/glass.png",
		"resources/textures/block/glass_pane_top.png",
		"resources/textures/block/bricks.png",
		"resources/textures/block/white_concrete.png"
	);

	self->textures[TEXTURE_UI_CROSSHAIR] = texture_create("resources/textures/gui/crosshair.png");
	self->textures[TEXTURE_UI_TOOLBAR] = texture_create("resources/textures/gui/toolbar.png");
	self->textures[TEXTURE_UI_TOOLBAR_HIGHLIGHT] = texture_create("resources/textures/gui/toolbar_highlight.png");

	self->textures[TEXTURE_CUBEMAP_SKYBOX] = texture_cubemap_create(
		"resources/textures/skybox/right.png",
		"resources/textures/skybox/left.png",
		"resources/textures/skybox/top.png",
		"resources/textures/skybox/bottom.png",
		"resources/textures/skybox/front.png",
		"resources/textures/skybox/back.png");
}

void renderer_update(Renderer* self, glm::mat4 perspective, glm::mat4 orthographic)
{
	self->projection_perspective = perspective;
	self->projection_ortho = orthographic;
}

Renderer renderer_create()
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	Renderer renderer;

	_load_shaders(&renderer);
	_load_textures(&renderer);

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
	shader_set_mat4(shader, ShaderUniform::PROJECTION, self->projection_ortho);
	shader_set_mat4(shader, ShaderUniform::VIEW, view);

	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(quad->position, 1));
	model = glm::scale(model, glm::vec3(quad->scale, 0));
	shader_set_mat4(shader, ShaderUniform::MODEL, model);

	if (texture != nullptr)
		shader_set_int(shader, 3, texture->handle);

	glBindVertexArray(quad->vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

// TODO: renderer_render_cube_texture
void renderer_render_cube(Renderer* self, glm::mat4 view, Cube* cube)
{
	ShaderProgram* shader = &self->shaders[cube->shader_type];

	shader_use(shader);
	shader_set_mat4(shader, ShaderUniform::PROJECTION, self->projection_perspective);
	shader_set_mat4(shader, ShaderUniform::VIEW, view);

	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, cube->position);
	model = glm::scale(model, cube->scale);
	shader_set_mat4(shader, ShaderUniform::MODEL, model);

	int a = GL_TEXTURE_CUBE_MAP;

	glBindVertexArray(cube->vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void renderer_render_custom(Renderer* self, glm::mat4 view, TextureType texture_type, ShaderType shader_type, int vao, int indicies, glm::vec3 position, glm::vec3 scale)
{
	ShaderProgram* shader = &self->shaders[shader_type];
	Texture* texture = &self->textures[texture_type];

	shader_use(shader);
	shader_set_mat4(shader, ShaderUniform::PROJECTION, self->projection_perspective);
	shader_set_mat4(shader, ShaderUniform::VIEW, view);

	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, position);
	model = glm::scale(model, scale);
	shader_set_mat4(shader, ShaderUniform::MODEL, model);

	glActiveTexture(GL_TEXTURE0 + texture->handle);
	glBindTexture(texture->type, texture->handle);

	glBindVertexArray(vao);

	glDrawArrays(GL_TRIANGLES, 0, indicies);
}
