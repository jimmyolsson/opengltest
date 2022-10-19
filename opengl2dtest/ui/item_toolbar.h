#pragma once

struct ItemToolbar
{
	unsigned int vao;
	unsigned int vbo;

	unsigned int svbo;
	unsigned int svao;

	short item_selected;
	shader_program sp;

	glm::dvec2 pos;
	glm::dvec2 toolbar_pos;

	unsigned int handle;
	unsigned int shandle;
};

void menu_loadtexture(unsigned int* handle, unsigned int* shandle)
{
	glGenTextures(1, handle);
	glBindTexture(GL_TEXTURE_2D, *handle);

	int width, height, nrChannels;
	unsigned char* data = stbi_load("..\\resources\\textures\\gui\\item_menu.png", &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenTextures(1, shandle);
	glBindTexture(GL_TEXTURE_2D, *shandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned char* sdata = stbi_load("..\\resources\\textures\\gui\\selected_item.png", &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, sdata);

	if (data)
	{
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	stbi_image_free(sdata);
}

void menu_render_hightlight(ItemToolbar* self, glm::mat4 p, glm::mat4 v)
{
}

void menu_render(ItemToolbar* self, float width, float height, glm::mat4 p, glm::mat4 v)
{
	shader_use(&self->sp);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, self->handle);

	glm::mat4 transform = glm::mat4(1);
	int menu_scale_x = 732;
	int menu_scale_y = 85;
	self->toolbar_pos = glm::vec2(width / 2 - menu_scale_x / 2, 0);
	transform = glm::translate(transform, glm::vec3(self->toolbar_pos, 1));
	transform = glm::scale(transform, glm::vec3(732, 85, 0));

	shader_set_mat4(&self->sp, "model", transform);
	shader_set_mat4(&self->sp, "projection", p);
	shader_set_mat4(&self->sp, "view", v);
	shader_set_int(&self->sp, "texture1", self->handle);

	glBindVertexArray(self->vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Render hightlight box
	shader_use(&self->sp);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, self->shandle);

	transform = glm::dmat4(1);
	// I have no idea why the offset on screen grows larger when the position diff stays the same :(
	int step = 80;
	transform = glm::translate(transform, glm::vec3(self->pos, 1));
	transform = glm::scale(transform, glm::vec3(98, 88, 0));

	shader_set_mat4(&self->sp, "model", transform);
	shader_set_mat4(&self->sp, "projection", p);
	shader_set_mat4(&self->sp, "view", v);
	shader_set_int(&self->sp, "texture1", self->shandle);

	glBindVertexArray(self->svao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void static quad_create(unsigned int* vao, unsigned int* vbo)
{
	int index = 0;
	std::vector<int> gpu_data;
	for (int i = 0; i < 30 - 1; i++)
	{
		gpu_data.push_back(m_back_verticies[i]);
	}
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, gpu_data.size() * sizeof(int), gpu_data.data(), GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 5 * BLOCK_SIZE_BYTES, (void*)0);
	glEnableVertexAttribArray(0);

	// Texture coord
	glVertexAttribPointer(1, 2, GL_UNSIGNED_INT, GL_FALSE, 5 * BLOCK_SIZE_BYTES, (void*)(3 * BLOCK_SIZE_BYTES));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

ItemToolbar menu_create()
{
	ItemToolbar menu;
	menu.item_selected = 0;

	shader_load(&menu.sp, "..\\resources\\shaders\\basic_texture_vert.glsl", GL_VERTEX_SHADER);
	shader_load(&menu.sp, "..\\resources\\shaders\\basic_texture_frag.glsl", GL_FRAGMENT_SHADER);
	shader_link(&menu.sp);

	quad_create(&menu.vao, &menu.vbo);
	quad_create(&menu.svao, &menu.svbo);

	menu_loadtexture(&menu.handle, &menu.shandle);

	return menu;
}

